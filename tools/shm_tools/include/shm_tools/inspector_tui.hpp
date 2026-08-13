#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <ncurses.h>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

struct QueueSnapshot
{
    std::string name;
    std::string proto_name;
    std::size_t buffered{0};
    std::size_t capacity{0};
    std::size_t sampled_messages{0};
    double sampled_rate{0.0};
    long long age_ms{-1};
    std::vector<std::string> messages;
};

struct InspectorSnapshot
{
    std::string shm_name;
    std::vector<QueueSnapshot> queues;
    std::size_t total_buffered{0};
    bool history{false};
};

class InspectorTui
{
public:

    InspectorTui()
    {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        timeout(0);
    }

    ~InspectorTui()
    {
        endwin();
    }

    InspectorTui(const InspectorTui&) = delete;
    InspectorTui& operator=(const InspectorTui&) = delete;

    bool process_input()
    {
        int key;

        while ((key = getch()) != ERR)
        {
            switch (key)
            {
                case 'q':
                case 'Q':
                    return false;

                case KEY_UP:
                case 'k':
                    if (selected_ > 0)
                        --selected_;
                    break;

                case KEY_DOWN:
                case 'j':
                    if (selected_ + 1 < visible_count_)
                        ++selected_;
                    break;

                case 'p':
                case 'P':
                    paused_ = !paused_;
                    break;

                case 'h':
                case 'H':
                    toggle_history_requested_ = true;
                    break;

                case '/':
                    prompt_filter();
                    break;

                case KEY_RESIZE:
                    break;

                default:
                    break;
            }
        }

        return true;
    }

    bool paused() const
    {
        return paused_;
    }

    bool consume_history_toggle()
    {
        if (!toggle_history_requested_)
            return false;

        toggle_history_requested_ = false;
        return true;
    }

    void draw(const InspectorSnapshot& snapshot)
    {
        erase();

        int height = 0;
        int width = 0;

        getmaxyx(stdscr, height, width);

        if (height < 12 || width < 70)
        {
            mvprintw(
                0,
                0,
                "Terminal too small (%dx%d). Minimum ~70x12.",
                width,
                height);

            refresh();
            return;
        }

        draw_header(snapshot, width);

        mvhline(1, 0, ACS_HLINE, width);

        const int left_width =
            std::clamp(width / 3, 30, 48);

        mvvline(
            2,
            left_width,
            ACS_VLINE,
            height - 3);

        const auto visible = filtered(snapshot);

        visible_count_ = visible.size();

        if (visible_count_ == 0)
        {
            selected_ = 0;
            scroll_ = 0;
        }
        else if (selected_ >= visible_count_)
        {
            selected_ = visible_count_ - 1;
        }

        draw_queue_list(
            visible,
            2,
            0,
            height - 3,
            left_width);

        draw_details(
            visible,
            2,
            left_width + 2,
            height - 3,
            width - left_width - 2);

        draw_footer(height, width);

        refresh();
    }

private:

    using QueueList = std::vector<const QueueSnapshot*>;

    std::size_t selected_{0};
    std::size_t scroll_{0};
    std::size_t visible_count_{0};

    bool paused_{false};
    bool toggle_history_requested_{false};

    std::string filter_;

    QueueList filtered(
        const InspectorSnapshot& snapshot) const
    {
        QueueList result;

        for (const auto& queue : snapshot.queues)
        {
            if (filter_.empty() ||
                queue.name.find(filter_) != std::string::npos ||
                queue.proto_name.find(filter_) != std::string::npos)
            {
                result.push_back(&queue);
            }
        }

        return result;
    }

    static std::string progress_bar(
        std::size_t value,
        std::size_t capacity,
        std::size_t width)
    {
        const std::size_t filled =
            capacity
                ? std::min(
                    width,
                    value * width / capacity)
                : 0;

        return std::string(filled, '#') +
               std::string(width - filled, '.');
    }

    static void print_clipped(
        int y,
        int x,
        std::string_view text,
        int max_width)
    {
        if (max_width <= 0)
            return;

        mvaddnstr(
            y,
            x,
            text.data(),
            std::min<int>(
                static_cast<int>(text.size()),
                max_width));
    }

    void draw_header(
        const InspectorSnapshot& snapshot,
        int width) const
    {
        std::ostringstream out;

        out
            << " Bridge Inspector"
            << " | "
            << snapshot.shm_name
            << " | "
            << snapshot.queues.size()
            << " queues"
            << " | "
            << snapshot.total_buffered
            << " buffered"
            << " | "
            << (snapshot.history ? "HISTORY" : "LATEST");

        if (paused_)
            out << " | PAUSED";

        if (!filter_.empty())
            out << " | filter=" << filter_;

        attron(A_BOLD);
        print_clipped(0, 0, out.str(), width);
        attroff(A_BOLD);
    }

    void draw_queue_list(
        const QueueList& queues,
        int y,
        int x,
        int height,
        int width)
    {
        attron(A_BOLD);
        print_clipped(
            y,
            x + 1,
            "QUEUE            USED      SAMPLE/s",
            width - 2);
        attroff(A_BOLD);

        const int available_rows = height - 1;

        if (available_rows <= 0)
            return;

        if (selected_ < scroll_)
            scroll_ = selected_;

        if (selected_ >= scroll_ +
                         static_cast<std::size_t>(available_rows))
        {
            scroll_ =
                selected_ -
                static_cast<std::size_t>(available_rows) +
                1;
        }

        for (int row = 0;
             row < available_rows;
             ++row)
        {
            const std::size_t index =
                scroll_ + static_cast<std::size_t>(row);

            if (index >= queues.size())
                break;

            const auto& queue = *queues[index];

            std::ostringstream line;

            line
                << std::left
                << std::setw(14)
                << queue.name.substr(0, 13)
                << ' '
                << std::setw(9)
                << (
                    std::to_string(queue.buffered) +
                    "/" +
                    std::to_string(queue.capacity))
                << ' '
                << std::fixed
                << std::setprecision(1)
                << queue.sampled_rate;

            if (index == selected_)
                attron(A_REVERSE);

            print_clipped(
                y + 1 + row,
                x + 1,
                line.str(),
                width - 2);

            if (index == selected_)
                attroff(A_REVERSE);
        }
    }

    void draw_details(
        const QueueList& queues,
        int y,
        int x,
        int height,
        int width) const
    {
        if (queues.empty() ||
            selected_ >= queues.size())
        {
            print_clipped(
                y,
                x,
                "No queue selected",
                width);

            return;
        }

        const QueueSnapshot& queue =
            *queues[selected_];

        attron(A_BOLD);
        print_clipped(
            y,
            x,
            queue.name,
            width);
        attroff(A_BOLD);

        print_clipped(
            y + 1,
            x,
            queue.proto_name,
            width);

        std::ostringstream stats;

        stats
            << queue.buffered
            << '/'
            << queue.capacity
            << " buffered  ["
            << progress_bar(
                queue.buffered,
                queue.capacity,
                12)
            << ']';

        print_clipped(
            y + 3,
            x,
            stats.str(),
            width);

        std::ostringstream rate;

        rate
            << "Samples: "
            << queue.sampled_messages
            << "  Sample rate: "
            << std::fixed
            << std::setprecision(1)
            << queue.sampled_rate
            << "/s  Last: ";

        if (queue.age_ms < 0)
            rate << "never";
        else
            rate << queue.age_ms << " ms";

        print_clipped(
            y + 4,
            x,
            rate.str(),
            width);

        mvhline(
            y + 5,
            x,
            ACS_HLINE,
            std::max(0, width - 1));

        if (queue.messages.empty())
        {
            print_clipped(
                y + 7,
                x,
                "(no message sampled)",
                width);

            return;
        }

        int row = y + 7;
        const int last_row = y + height;

        //
        // En latest -> 1 message.
        // En history -> on affiche les messages
        // successivement jusqu'en bas de l'écran.
        //
        for (std::size_t i = 0;
             i < queue.messages.size() &&
             row < last_row;
             ++i)
        {
            if (queue.messages.size() > 1)
            {
                std::ostringstream title;
                title << "Message #" << i;

                attron(A_BOLD);
                print_clipped(
                    row++,
                    x,
                    title.str(),
                    width);
                attroff(A_BOLD);
            }

            std::istringstream input(
                queue.messages[i]);

            std::string line;

            while (std::getline(input, line) &&
                   row < last_row)
            {
                print_clipped(
                    row++,
                    x,
                    line,
                    width);
            }

            if (row < last_row)
                ++row;
        }
    }

    void draw_footer(
        int height,
        int width) const
    {
        std::string footer =
            " q quit | up/down select | p pause | "
            "h latest/history | / filter ";

        attron(A_REVERSE);
        mvhline(
            height - 1,
            0,
            ' ',
            width);

        print_clipped(
            height - 1,
            0,
            footer,
            width);

        attroff(A_REVERSE);
    }

    void prompt_filter()
    {
        int height = 0;
        int width = 0;

        getmaxyx(stdscr, height, width);

        timeout(-1);
        echo();
        curs_set(1);

        move(height - 1, 0);
        clrtoeol();

        mvprintw(
            height - 1,
            0,
            "Filter (empty = all): ");

        char buffer[256]{};

        getnstr(
            buffer,
            static_cast<int>(
                sizeof(buffer) - 1));

        filter_ = buffer;

        selected_ = 0;
        scroll_ = 0;

        noecho();
        curs_set(0);
        timeout(0);
    }
};