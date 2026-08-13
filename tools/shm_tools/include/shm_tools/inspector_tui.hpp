#pragma once

#include "shm_tools/inspector_types.hpp"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <ncurses.h>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

class InspectorTui {
public:
  enum class ActionKind {
    None,
    Redraw,
    Quit,
    NextSource,
    PreviousSource,
    SelectSource,
    ToggleHistory
  };

  struct Action {
    ActionKind kind{ActionKind::None};
    std::size_t source_index{0};
  };

  InspectorTui() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(0);
  }

  ~InspectorTui() { endwin(); }

  InspectorTui(const InspectorTui &) = delete;
  InspectorTui &operator=(const InspectorTui &) = delete;

  bool paused() const noexcept { return paused_; }

  void reset_selection() {
    selected_ = 0;
    queue_scroll_ = 0;
    detail_scroll_ = 0;
  }

  Action process_input(std::size_t source_count) {
    const int key = getch();

    if (key == ERR)
      return {};

    switch (key) {
    case 'q':
    case 'Q':
      return {ActionKind::Quit, 0};

    case '\t':
      return {ActionKind::NextSource, 0};

#ifdef KEY_BTAB
    case KEY_BTAB:
      return {ActionKind::PreviousSource, 0};
#endif

    case 'p':
    case 'P':
      paused_ = !paused_;
      return {ActionKind::Redraw, 0};

    case 'h':
    case 'H':
      return {ActionKind::ToggleHistory, 0};

    case '/':
      prompt_filter();
      return {ActionKind::Redraw, 0};

    case 'c':
    case 'C':
      filter_.clear();
      reset_selection();
      return {ActionKind::Redraw, 0};

    case KEY_UP:
    case 'k':
      if (selected_ > 0) {
        --selected_;
        detail_scroll_ = 0;
      }
      return {ActionKind::Redraw, 0};

    case KEY_DOWN:
    case 'j':
      if (selected_ + 1 < visible_count_) {
        ++selected_;
        detail_scroll_ = 0;
      }
      return {ActionKind::Redraw, 0};

    case KEY_PPAGE:
      if (detail_scroll_ > 10)
        detail_scroll_ -= 10;
      else
        detail_scroll_ = 0;
      return {ActionKind::Redraw, 0};

    case KEY_NPAGE:
      detail_scroll_ += 10;
      return {ActionKind::Redraw, 0};

    case KEY_HOME:
      detail_scroll_ = 0;
      return {ActionKind::Redraw, 0};

    default:
      break;
    }

    if (key >= '1' && key <= '9') {
      const auto index = static_cast<std::size_t>(key - '1');

      if (index < source_count)
        return {ActionKind::SelectSource, index};
    }

    return {};
  }

  void draw(const InspectorSnapshot &snapshot,
            const std::vector<std::string> &source_labels,
            std::size_t active_source, int rate_hz) {
    erase();

    int height = 0;
    int width = 0;
    getmaxyx(stdscr, height, width);

    if (height < 12 || width < 72) {
      mvprintw(0, 0, "Terminal too small (%dx%d). Minimum: 72x12.", width,
               height);
      refresh();
      return;
    }

    draw_top_bar(snapshot, source_labels, active_source, rate_hz, width);

    draw_status_bar(snapshot, width);
    mvhline(2, 0, ACS_HLINE, width);

    const int left_width = std::clamp(width / 3, 30, 48);
    const int body_y = 3;
    const int body_height = height - body_y - 1;

    mvvline(body_y, left_width, ACS_VLINE, body_height);

    const auto visible = filtered(snapshot);
    visible_count_ = visible.size();

    if (visible.empty()) {
      selected_ = 0;
      queue_scroll_ = 0;
      detail_scroll_ = 0;
    } else if (selected_ >= visible.size()) {
      selected_ = visible.size() - 1;
      detail_scroll_ = 0;
    }

    draw_queue_list(visible, body_y, 0, body_height, left_width);

    draw_details(visible, body_y, left_width + 2, body_height,
                 width - left_width - 2, snapshot.error);

    draw_footer(height, width);
    refresh();
  }

private:
  using QueueList = std::vector<const QueueSnapshot *>;

  static void print_clipped(int y, int x, std::string_view text,
                            int max_width) {
    if (max_width <= 0)
      return;

    mvaddnstr(y, x, text.data(),
              std::min<int>(static_cast<int>(text.size()), max_width));
  }

  static std::string bar(std::size_t value, std::size_t capacity,
                         std::size_t width) {
    const auto filled =
        capacity ? std::min(width, value * width / capacity) : std::size_t{0};

    return std::string(filled, '#') + std::string(width - filled, '.');
  }

  QueueList filtered(const InspectorSnapshot &snapshot) const {
    QueueList result;

    for (const auto &queue : snapshot.queues) {
      if (filter_.empty() || queue.name.find(filter_) != std::string::npos ||
          queue.proto_name.find(filter_) != std::string::npos) {
        result.push_back(&queue);
      }
    }

    return result;
  }

  void draw_top_bar(const InspectorSnapshot &snapshot,
                    const std::vector<std::string> &labels, std::size_t active,
                    int rate_hz, int width) const {
    int x = 0;

    attron(A_BOLD);
    print_clipped(0, x, " SHM Inspector ", width);
    attroff(A_BOLD);
    x += 15;

    for (std::size_t i = 0; i < labels.size() && x < width - 18; ++i) {
      std::ostringstream tab;
      tab << ' ' << (i + 1) << ':' << labels[i] << ' ';

      if (i == active)
        attron(A_REVERSE | A_BOLD);

      print_clipped(0, x, tab.str(), width - x);

      if (i == active)
        attroff(A_REVERSE | A_BOLD);

      x += static_cast<int>(tab.str().size());
    }

    std::ostringstream right;
    right << rate_hz << "Hz | " << (snapshot.history ? "HISTORY" : "LATEST");

    if (paused_)
      right << " | PAUSED";

    const auto text = right.str();
    const int right_x = std::max(0, width - static_cast<int>(text.size()) - 1);
    print_clipped(0, right_x, text, width - right_x);
  }

  void draw_status_bar(const InspectorSnapshot &snapshot, int width) const {
    std::ostringstream out;

    out << " SHM: " << snapshot.shm_name << " | " << snapshot.queues.size()
        << " queues"
        << " | " << snapshot.total_buffered << " buffered";

    if (!filter_.empty())
      out << " | filter=" << filter_;

    print_clipped(1, 0, out.str(), width);
  }

  void draw_queue_list(const QueueList &queues, int y, int x, int height,
                       int width) {
    attron(A_BOLD);
    print_clipped(y, x + 1, "QUEUE            USED       OBS/s", width - 2);
    attroff(A_BOLD);

    const int rows = height - 1;
    if (rows <= 0)
      return;

    if (selected_ < queue_scroll_)
      queue_scroll_ = selected_;

    if (selected_ >= queue_scroll_ + static_cast<std::size_t>(rows)) {
      queue_scroll_ = selected_ - static_cast<std::size_t>(rows) + 1;
    }

    for (int row = 0; row < rows; ++row) {
      const auto index = queue_scroll_ + static_cast<std::size_t>(row);

      if (index >= queues.size())
        break;

      const auto &queue = *queues[index];

      std::ostringstream line;
      line << std::left << std::setw(15) << queue.name.substr(0, 14)
           << std::setw(11)
           << (std::to_string(queue.buffered) + "/" +
               std::to_string(queue.capacity))
           << std::fixed << std::setprecision(1) << queue.observed_rate;

      if (index == selected_)
        attron(A_REVERSE);

      print_clipped(y + 1 + row, x + 1, line.str(), width - 2);

      if (index == selected_)
        attroff(A_REVERSE);
    }
  }

  static std::vector<std::string> message_lines(const QueueSnapshot &queue) {
    std::vector<std::string> lines;

    for (std::size_t i = 0; i < queue.messages.size(); ++i) {
      if (queue.messages.size() > 1) {
        lines.push_back("--- Message #" + std::to_string(i) + " ---");
      }

      std::istringstream input(queue.messages[i]);
      std::string line;

      while (std::getline(input, line))
        lines.push_back(std::move(line));

      if (i + 1 != queue.messages.size())
        lines.emplace_back();
    }

    return lines;
  }

  void draw_details(const QueueList &queues, int y, int x, int height,
                    int width, const std::string &error) {
    if (!error.empty()) {
      attron(A_BOLD);
      print_clipped(y, x, "Shared memory error", width);
      attroff(A_BOLD);
      print_clipped(y + 2, x, error, width);
      return;
    }

    if (queues.empty() || selected_ >= queues.size()) {
      print_clipped(y, x, "No queue selected", width);
      return;
    }

    const auto &queue = *queues[selected_];

    attron(A_BOLD);
    print_clipped(y, x, queue.name, width);
    attroff(A_BOLD);

    print_clipped(y + 1, x, queue.proto_name, width);

    std::ostringstream usage;
    usage << queue.buffered << '/' << queue.capacity << "  ["
          << bar(queue.buffered, queue.capacity, 16) << ']';
    print_clipped(y + 3, x, usage.str(), width);

    std::ostringstream stats;
    stats << "Observed: " << queue.observed_messages
          << " | OBS/s: " << std::fixed << std::setprecision(1)
          << queue.observed_rate << " | Last: ";

    if (queue.age_ms < 0)
      stats << "never";
    else
      stats << queue.age_ms << " ms";

    print_clipped(y + 4, x, stats.str(), width);
    mvhline(y + 5, x, ACS_HLINE, std::max(0, width - 1));

    if (queue.messages.empty()) {
      print_clipped(y + 7, x, "(no message sampled)", width);
      detail_scroll_ = 0;
      return;
    }

    const auto lines = message_lines(queue);
    const int first_row = y + 7;
    const int visible_rows = std::max(0, height - 7);

    const std::size_t max_scroll =
        lines.size() > static_cast<std::size_t>(visible_rows)
            ? lines.size() - static_cast<std::size_t>(visible_rows)
            : 0;

    detail_scroll_ = std::min(detail_scroll_, max_scroll);

    for (int row = 0; row < visible_rows; ++row) {
      const auto line_index = detail_scroll_ + static_cast<std::size_t>(row);

      if (line_index >= lines.size())
        break;

      print_clipped(first_row + row, x, lines[line_index], width);
    }
  }

  void draw_footer(int height, int width) const {
    const std::string footer =
        " Tab/Shift-Tab SHM | 1-9 select | up/down queue | PgUp/PgDn detail | "
        "p pause | h history | / filter | c clear | q quit ";

    attron(A_REVERSE);
    mvhline(height - 1, 0, ' ', width);
    print_clipped(height - 1, 0, footer, width);
    attroff(A_REVERSE);
  }

  void prompt_filter() {
    int height = 0;
    int width = 0;
    getmaxyx(stdscr, height, width);
    (void)width;

    timeout(-1);
    echo();
    curs_set(1);

    move(height - 1, 0);
    clrtoeol();
    mvprintw(height - 1, 0, "Filter (empty = all): ");

    char buffer[256]{};
    getnstr(buffer, static_cast<int>(sizeof(buffer) - 1));

    filter_ = buffer;
    reset_selection();

    noecho();
    curs_set(0);
    timeout(0);
  }

  std::size_t selected_{0};
  std::size_t queue_scroll_{0};
  std::size_t detail_scroll_{0};
  std::size_t visible_count_{0};

  bool paused_{false};
  std::string filter_;
};