
#include <algorithm>
#include <array>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <pthread.h>
#include <ranges>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>
#if __has_include(<format>)
#include <format>
#define HAS_STD_FORMAT 1
#else
#define HAS_STD_FORMAT 0
#endif
typedef struct proc_data
{

    std::string std_out;
    int pid;
    int exit_code;

} p_data;

namespace
{
template <typename T> std::ostream &operator<<(std::ostream &os, std::vector<T> &v)
{
    if (v.size() == 0)
    {
        os << "[]";
        return os;
    }
    auto it{v.begin()};

    os << '[';
    while (it != v.end() - 1)
    {
        os << *it;
        os << ", \n\n";

        it++;
    }
    os << *it;
    os << ']';

    return os;
}
static constexpr std::string_view delim{"><node "};
static constexpr std::string_view delim_csv{"\",\""};

using node_types_strings_only = std::array<std::string_view, 18>;
static constexpr node_types_strings_only split_at_strings_strings_only{
    "NAF=\"",          "bounds=\"",   "checkable=\"",   "checked=\"",    "class=\"",    "clickable=\"",
    "content-desc=\"", "enabled=\"",  "focusable=\"",   "focused=\"",    "index=\"",    "long-clickable=\"",
    "package=\"",      "password=\"", "resource-id=\"", "scrollable=\"", "selected=\"", "text=\"",
};
node_types_strings_only::const_iterator nodes_sv_begin{split_at_strings_strings_only.cbegin()};
node_types_strings_only::const_iterator nodes_sv_end{split_at_strings_strings_only.cend()};

using node_types_strings_only_no_index = std::array<std::string_view, 17>;
static constexpr node_types_strings_only_no_index split_at_strings_strings_only_no_index{
    " NAF=\"",          " bounds=\"",      " checkable=\"",  " checked=\"",  " class=\"",          " clickable=\"",
    " content-desc=\"", " enabled=\"",     " focusable=\"",  " focused=\"",  " long-clickable=\"", " package=\"",
    " password=\"",     " resource-id=\"", " scrollable=\"", " selected=\"", " text=\"",
};
node_types_strings_only_no_index::const_iterator nodes_sv_no_index_begin{
    split_at_strings_strings_only_no_index.cbegin()};
node_types_strings_only_no_index::const_iterator nodes_sv_no_index_end{split_at_strings_strings_only_no_index.cend()};

typedef struct element_data
{

    std::string bounds;
    std::string text;
    std::string package;
    std::string resource_id;
    std::string clazz;
    std::string content_desc;

    int aa_center_x;
    int aa_center_y;
    int aa_area;
    int aa_width;
    int aa_height;
    int aa_start_x;
    int aa_start_y;
    int aa_end_x;
    int aa_end_y;
    int aa_is_square;
    float aa_w_h_relation;
    int checkable;
    int checked;
    int clickable;
    int enabled;
    int focusable;
    int focused;
    int index;
    int long_clickable;
    int password;
    int scrollable;
    int selected;
    int naf;

} el_data;

static constexpr std::string_view csv_header{
    "\"bounds\",\"text\",\"package\",\"resource_id\",\"clazz\",\"content_desc\",\"aa_center_x\",\"aa_center_y\",\"aa_"
    "area\",\"aa_width\",\"aa_height\",\"aa_start_x\",\"aa_start_y\",\"aa_end_x\",\"aa_end_y\",\"aa_is_square\",\"aa_w_"
    "h_relation\",\"checkable\",\"checked\",\"clickable\",\"enabled\",\"focusable\",\"focused\",\"index\",\"long_"
    "clickable\",\"password\",\"scrollable\",\"selected\",\"naf\""};

// for debug
std::ostream &operator<<(std::ostream &os, element_data &v)
{
    os << "ELEMENT:\n";
    os << "bounds: " << v.bounds << "\n";
    os << "text: " << v.text << "\n";
    os << "package: " << v.package << "\n";
    os << "resource_id: " << v.resource_id << "\n";
    os << "clazz: " << v.clazz << "\n";
    os << "content_desc: " << v.content_desc << "\n";
    os << "aa_center_x: " << v.aa_center_x << "\n";
    os << "aa_center_y: " << v.aa_center_y << "\n";
    os << "aa_area: " << v.aa_area << "\n";
    os << "aa_width: " << v.aa_width << "\n";
    os << "aa_height: " << v.aa_height << "\n";
    os << "aa_start_x: " << v.aa_start_x << "\n";
    os << "aa_start_y: " << v.aa_start_y << "\n";
    os << "aa_end_x: " << v.aa_end_x << "\n";
    os << "aa_end_y: " << v.aa_end_y << "\n";
    os << "aa_is_square: " << v.aa_is_square << "\n";
    os << "aa_w_h_relation: " << v.aa_w_h_relation << "\n";
    os << "checkable: " << v.checkable << "\n";
    os << "checked: " << v.checked << "\n";
    os << "clickable: " << v.clickable << "\n";
    os << "enabled: " << v.enabled << "\n";
    os << "focusable: " << v.focusable << "\n";
    os << "focused: " << v.focused << "\n";
    os << "index: " << v.index << "\n";
    os << "long_clickable: " << v.long_clickable << "\n";
    os << "password: " << v.password << "\n";
    os << "scrollable: " << v.scrollable << "\n";
    os << "selected: " << v.selected << "\n";
    os << "naf: " << v.naf << "\n\n";

    return os;
}

static constexpr std::string_view bool_true{"true"};
static constexpr std::string_view bool_true1{"true\""};
static constexpr std::string_view sv_NAF{"NAF=\""};
static constexpr std::string_view sv_bounds{"bounds=\""};
static constexpr std::string_view sv_checkable{"checkable=\""};
static constexpr std::string_view sv_checked{"checked=\""};
static constexpr std::string_view sv_class{"class=\""};
static constexpr std::string_view sv_clickable{"clickable=\""};
static constexpr std::string_view sv_content_desc{"content-desc=\""};
static constexpr std::string_view sv_enabled{"enabled=\""};
static constexpr std::string_view sv_focusable{"focusable=\""};
static constexpr std::string_view sv_focused{"focused=\""};
static constexpr std::string_view sv_index{"index=\""};
static constexpr std::string_view sv_long_clickable{"long-clickable=\""};
static constexpr std::string_view sv_package{"package=\""};
static constexpr std::string_view sv_password{"password=\""};
static constexpr std::string_view sv_resource_id{"resource-id=\""};
static constexpr std::string_view sv_scrollable{"scrollable=\""};
static constexpr std::string_view sv_selected{"selected=\""};
static constexpr std::string_view sv_text{"text=\""};

int static constexpr return_bool_as_int(std::string_view s)
{
    if ((s == bool_true) || (s == bool_true1))
    {
        return 1;
    }
    return 0;
}

void static replace_space_with_new_line(std::string &haystack)
{
    for (const auto &tag : split_at_strings_strings_only_no_index)
    {
        std::replace(haystack.begin(), haystack.end(), tag[0], '\n');
    }
}

static void const convert_bounds_to_array(std::string_view bounds, std::array<int, 4> &bounds_array)
{
    std::array<size_t, 2> open_brackets_indices{};
    std::array<size_t, 2> closed_brackets_indices{};
    std::array<size_t, 2> comma_indices{};

    size_t index_open_brackets_indices{0};
    size_t index_closed_brackets_indices{0};
    size_t index_comma_indices{0};

    for (size_t i{}; i < bounds.size(); i++)
    {

        if (bounds[i] == '[')
            open_brackets_indices[index_open_brackets_indices++] = i;
        else if (bounds[i] == ']')
            closed_brackets_indices[index_closed_brackets_indices++] = i;
        else if (bounds[i] == ',')
            comma_indices[index_comma_indices++] = i;
    }
    std::string_view first_number{
        bounds.substr(open_brackets_indices[0] + 1, comma_indices[0] - open_brackets_indices[0] - 1)};
    std::string_view second_number{
        bounds.substr(comma_indices[0] + 1, closed_brackets_indices[0] - comma_indices[0] - 1)};
    std::string_view third_number{
        bounds.substr(open_brackets_indices[1] + 1, comma_indices[1] - open_brackets_indices[1] - 1)};
    std::string_view fourth_number{
        bounds.substr(comma_indices[1] + 1, closed_brackets_indices[1] - comma_indices[1] - 1)};
    bounds_array[0] = std::stoi(first_number.data());
    bounds_array[1] = std::stoi(second_number.data());
    bounds_array[2] = std::stoi(third_number.data());
    bounds_array[3] = std::stoi(fourth_number.data());
}
void static parse_results(const std::string_view s, element_data &e)
{
    size_t indexofstring = 0;

    auto strs = s | std::views::split('\n');
    for (const auto &ref : strs)
    {
        node_types_strings_only::const_iterator it{nodes_sv_begin};
        std::string_view r{ref.begin(), ref.end()};
        while (it != nodes_sv_end)
        {
            indexofstring = r.find(*it);

            if ((indexofstring == 0) && (*it == sv_bounds))
            {
                auto rsubs{r.substr(it->size(), r.size() - it->size() - 1)};
                auto rsubssplit = rsubs | std::views::split('"');
                for (const auto &refu : rsubssplit)
                {
                    e.bounds = std::string{refu.begin(), refu.end()};
                    break;
                }
                std::array<int, 4> bounds_array{};
                convert_bounds_to_array(e.bounds, bounds_array);
                e.aa_start_x = bounds_array[0];
                e.aa_start_y = bounds_array[1];
                e.aa_end_x = bounds_array[2];
                e.aa_end_y = bounds_array[3];
                e.aa_center_y = (e.aa_start_y + e.aa_end_y) / 2;
                e.aa_area = (e.aa_end_x - e.aa_start_x) * (e.aa_end_y - e.aa_start_y);
                e.aa_width = e.aa_end_x - e.aa_start_x;
                e.aa_height = e.aa_end_y - e.aa_start_y;
                e.aa_center_x = (e.aa_start_x + e.aa_end_x) / 2;
                e.aa_is_square = (e.aa_width == e.aa_height);
                if (e.aa_height > 0)
                {
                    e.aa_w_h_relation = (float)e.aa_width / e.aa_height;
                }
            }
            else if ((indexofstring == 0) && (*it == sv_checkable))
            {
                e.checkable = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_checked))
            {
                e.checked = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_class))
            {
                e.clazz = r.substr(it->size(), r.size() - it->size() - 1);
            }
            else if ((indexofstring == 0) && (*it == sv_clickable))
            {
                e.clickable = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_content_desc))
            {
                e.content_desc = r.substr(it->size(), r.size() - it->size());
                if (e.content_desc.back() == '"')
                {
                    e.content_desc.pop_back();
                }
            }
            else if ((indexofstring == 0) && (*it == sv_enabled))
            {
                e.enabled = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_focusable))
            {
                e.focusable = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_focused))
            {
                e.focused = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_index))
            {
                e.index = std::stoi(r.substr(it->size(), r.size() - it->size() - 1).data());
            }
            else if ((indexofstring == 0) && (*it == sv_long_clickable))
            {
                e.long_clickable = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_package))
            {
                e.package = r.substr(it->size(), r.size() - it->size() - 1);
            }
            else if ((indexofstring == 0) && (*it == sv_password))
            {
                e.password = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_resource_id))
            {
                e.resource_id = r.substr(it->size(), r.size() - it->size() - 1);
            }
            else if ((indexofstring == 0) && (*it == sv_scrollable))
            {
                e.scrollable = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_selected))
            {
                e.selected = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            else if ((indexofstring == 0) && (*it == sv_text))
            {
                e.text = r.substr(it->size(), r.size() - it->size());
                if (e.text.back() == '"')
                {
                    e.text.pop_back();
                }
            }
            else if ((indexofstring == 0) && (*it == sv_NAF))
            {
                e.naf = return_bool_as_int(r.substr(it->size(), r.size() - it->size() - 1));
            }
            it++;
        }
    }
}
} // namespace

namespace uiautomator_dumper
{
std::string dump_struct_vector_as_csv(std::vector<element_data> &v)
{
    std::string outputstring;
    if (!v.empty())
    {
        size_t maxsize{sizeof(v[0])};
        for (const auto &ref : v)
        {
            if (sizeof(ref) > maxsize)
            {
                maxsize = sizeof(ref);
            }
        }
        outputstring.reserve(v.size() * maxsize * 2);
        outputstring.append(csv_header);
        outputstring += '\n';
    }
    else
    {
        return outputstring;
    }

    std::vector<element_data>::iterator it{v.begin()};
    while (it != v.end())
    {
#if HAS_STD_FORMAT
        outputstring.append(std::format(
            "\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\","
            "\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\",\"{}\"\n",
            it->bounds, it->text, it->package, it->resource_id, it->clazz, it->content_desc, it->aa_center_x,
            it->aa_center_y, it->aa_area, it->aa_width, it->aa_height, it->aa_start_x, it->aa_start_y, it->aa_end_x,
            it->aa_end_y, it->aa_is_square, it->aa_w_h_relation, it->checkable, it->checked, it->clickable, it->enabled,
            it->focusable, it->focused, it->index, it->long_clickable, it->password, it->scrollable, it->selected,
            it->naf));
#else
        outputstring += '"';
        outputstring.append(it->bounds);
        outputstring.append(delim_csv);
        outputstring.append(it->text);
        outputstring.append(delim_csv);

        outputstring.append(it->package);
        outputstring.append(delim_csv);

        outputstring.append(it->resource_id);
        outputstring.append(delim_csv);

        outputstring.append(it->clazz);
        outputstring.append(delim_csv);

        outputstring.append(it->content_desc);
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_center_x));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_center_y));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_area));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_width));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_height));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_start_x));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_start_y));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_end_x));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_end_y));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_is_square));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->aa_w_h_relation));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->checkable));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->checked));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->clickable));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->enabled));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->focusable));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->focused));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->index));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->long_clickable));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->password));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->scrollable));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->selected));
        outputstring.append(delim_csv);

        outputstring.append(std::to_string(it->naf));
        outputstring += '"';
        outputstring += '\n';

#endif
        it++;
    }
    return outputstring;
}

static std::vector<element_data> parse_uiautomator_dump(std::string &dumpstring)
{
    for (size_t i{}; i < dumpstring.size(); i++)
    {
        if (dumpstring[i] == '\n' || dumpstring[i] == '\r')
        {
            dumpstring[i] = ' ';
        }
    }
    std::string_view dumpstringview{dumpstring};
    auto strings = dumpstringview | std::views::split(delim);
    std::vector<std::string> splitstrings;
    splitstrings.reserve(1024);
    size_t indexcounter{};
    size_t pullcounter{};
    for (const auto &ref : strings)
    {
        if (pullcounter == 0)
        {
            pullcounter++;
            continue;
        }
        splitstrings.emplace_back(std::string{ref.begin(), ref.end()});
        replace_space_with_new_line(splitstrings[indexcounter]);
        indexcounter++;
    }
    std::vector<element_data> parsed_elements;
    parsed_elements.resize(splitstrings.size());
    for (size_t i{}; i < parsed_elements.size(); i++)
    {
        parse_results(splitstrings[i], parsed_elements[i]);
    }
    return parsed_elements;
}
} // namespace uiautomator_dumper

int parse_window_dump_xml()
{
    std::string file_to_open = "/sdcard/window_dump.xml";
    std::ifstream ifs;
    ifs.open(file_to_open, std::ifstream::in);
    if (!ifs.is_open())
    {
        std::cerr << "Error opening file: " << file_to_open << '\n';
        return 1;
    }
    std::string dumpstring;
    dumpstring.reserve(40960);
    std::string dumpstringgetline;
    dumpstring.reserve(40960);

    while (std::getline(ifs, dumpstringgetline))
    {
        dumpstring.append(std::move(dumpstringgetline));
    }
    ifs.close();
    std::vector<element_data> parsed_elements{uiautomator_dumper::parse_uiautomator_dump(dumpstring)};
    std::string csv_data_outout{uiautomator_dumper::dump_struct_vector_as_csv(parsed_elements)};
    std::cout << csv_data_outout;
    return 0;
}
// for debug
std::ostream &operator<<(std::ostream &os, proc_data &v)
{
    os << "STDOUT:\n";
    os << v.std_out << "\n";
    os << "PID:\n";
    os << v.pid << "\n\n";
    os << "EXIT CODE:\n";
    os << v.exit_code << "\n\n";
    return os;
}

pid_t system2(const char *command, int *infp, int *outfp)
{
    int p_stdin[2];
    int p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) == -1)
        return -1;

    if (pipe(p_stdout) == -1)
    {
        close(p_stdin[0]);
        close(p_stdin[1]);
        return -1;
    }

    pid = fork();

    if (pid < 0)
    {
        close(p_stdin[0]);
        close(p_stdin[1]);
        close(p_stdout[0]);
        close(p_stdout[1]);
        return pid;
    }
    else if (pid == 0)
    {
        close(p_stdin[1]);
        dup2(p_stdin[0], 0);
        close(p_stdout[0]);
        dup2(p_stdout[1], 1);
        dup2(::open("/dev/null", O_RDONLY), 2);
        for (int i = 3; i < 4096; ++i)
            ::close(i);
        setsid();
        execl("/bin/sh", "sh", "-c", command, NULL);
        _exit(1);
    }
    close(p_stdin[0]);
    close(p_stdout[1]);

    if (infp == NULL)
    {
        close(p_stdin[1]);
    }
    else
    {
        *infp = p_stdin[1];
    }

    if (outfp == NULL)
    {
        close(p_stdout[0]);
    }
    else
    {
        *outfp = p_stdout[0];
    }

    return pid;
}

proc_data execute_command_capture_stdout_and_wait(const char *command)
{
    int out_fd;
    auto pid = system2(command, NULL, &out_fd);
    if (pid < 0)
    {
        throw std::runtime_error("Failed to execute command");
    }
    char buffer[128];
    ssize_t bytes_read;
    std::string output;
    output.reserve(1024);
    while ((bytes_read = read(out_fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        for (int i = 0; i < bytes_read; i++)
        {
            output += buffer[i];
            buffer[i] = '\0';
        }
    }
    close(out_fd);
    int status;
    waitpid(pid, &status, 0);
    return proc_data{output, pid, status};
}
int execute_command_no_wait_return_pid(const char *command)
{
    int out_fd;
    auto pid = system2(command, NULL, NULL);
    if (pid < 0)
    {
        throw std::runtime_error("Failed to execute command");
    }
    return pid;
}

int execute_command_wait_return_pid(const char *command)
{
    int out_fd;
    auto pid = system2(command, NULL, NULL);
    if (pid < 0)
    {
        throw std::runtime_error("Failed to execute command");
    }
    int status;
    waitpid(pid, &status, 0);
    return pid;
}

const char *get_pid_from_top_activity_cmd = "/system/bin/dumpsys activity top | grep -F \"ACTIVITY\" | tail -n1";

int get_pid_from_top_activity()
{
    auto result{execute_command_capture_stdout_and_wait(get_pid_from_top_activity_cmd)};
    if (result.exit_code != 0)
    {
        throw std::runtime_error("Failed to execute dumpsys command");
    }
    auto strs = result.std_out | std::views::split(' ');
    for (const auto &ref : strs)
    {
        std::string_view r{ref.begin(), ref.end()};
        if (r.size() > 4 && r.substr(0, 4) == "pid=")
        {
            return std::stoi(std::string(r.substr(4)));
        }
    }
    return -1;
}

std::string cpu_limit_pid(int pid, int limit = 1, bool include_children = true)
{
    std::string pid_str{std::to_string(pid)};
    std::string limit_str{std::to_string(limit)};
    std::string limit_cmd;
    if (include_children)
    {
        limit_cmd = "cpulimit --limit=" + limit_str + " --include-children --pid=" + pid_str;
    }
    else
    {
        limit_cmd = "cpulimit --limit=" + limit_str + " --pid=" + pid_str;
    }
    int pid_as_int{execute_command_no_wait_return_pid(limit_cmd.c_str())};
    std::string pid_as_string{"top -b -n1 | grep -F \"" + limit_cmd +
                              "\" | grep -vF \"grep\" | grep -vF \"sh -c\""
                              "| awk '{print $1}'"};
    auto result{execute_command_capture_stdout_and_wait(pid_as_string.c_str())};
    std::string rawstring;
    rawstring.reserve(6);
    for (size_t i{}; i < result.std_out.size(); i++)
    {
        if (result.std_out[i] >= '0' && result.std_out[i] <= '9')
        {
            rawstring += result.std_out[i];
        }
    }
    return rawstring;
}
void sleep_ms(int milliseconds)
{
    usleep(milliseconds * 1000);
}

int main(int argc, char *argv[])
{
    int limit_top_activity_to{1};
    if (argc > 1)
    {
        limit_top_activity_to = std::stoi(argv[1]);
    }
    if (limit_top_activity_to < 0)
    {
        limit_top_activity_to = 100;
    }
    int priority{0};
    if (argc > 2)
    {
        priority = std::stoi(argv[2]);
    }
    if (priority < 0)
    {
        priority = ::abs(priority);
    }
    if (priority > 19)
    {
        priority = 19;
    }
    std::string nicecmd{"nice --" + std::to_string(priority) + " /system/bin/uiautomator dump"};
    int pid_from_top_activity{get_pid_from_top_activity()};
    std::string pid_of_cpulimit_string{cpu_limit_pid(pid_from_top_activity, limit_top_activity_to, true)};
    // system("nice --adjustment=-20 /system/bin/uiautomator dump");
    system(nicecmd.c_str());
    parse_window_dump_xml();
    std::string killchars{"kill"};
    char *args[] = {(char *)killchars.c_str(), (char *)pid_of_cpulimit_string.c_str(), NULL};
    execvp("kill", args);
    return 0;
}
