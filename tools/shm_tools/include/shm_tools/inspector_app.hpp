#pragma once

#include "shm_tools/bridge_inspector.hpp"
#include "shm_tools/inspector_tui.hpp"
#include "shm_tools/inspector_types.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class InspectorApp {
public:
  template <typename Inspector>
  void add(std::string label, std::string shm_name) {
    sources_.push_back(
        {std::move(label), std::make_unique<Inspector>(std::move(shm_name))});
  }

  int run(InspectorOptions options) {
    if (sources_.empty())
      throw std::runtime_error("No shared memory source configured");

    options.rate = std::max(options.rate, 1);

    std::vector<InspectorSnapshot> snapshots(sources_.size());
    std::vector<bool> has_snapshot(sources_.size(), false);

    for (std::size_t i = 0; i < sources_.size(); ++i)
      snapshots[i].shm_name = std::string(sources_[i].inspector->shm_name());

    std::vector<std::string> labels;
    labels.reserve(sources_.size());

    for (const auto &source : sources_)
      labels.push_back(source.label);

    InspectorTui tui;

    const auto period = std::chrono::milliseconds(1000 / options.rate);

    std::size_t active = 0;
    bool running = true;
    bool force_poll = true;
    auto next_poll = std::chrono::steady_clock::now();

    while (running) {
      const auto now = std::chrono::steady_clock::now();

      if (force_poll || !has_snapshot[active] ||
          (!tui.paused() && now >= next_poll)) {
        poll_source(active, options.history, snapshots, has_snapshot);
        force_poll = false;
        next_poll = std::chrono::steady_clock::now() + period;
      }

      tui.draw(snapshots[active], labels, active, options.rate);

      for (;;) {
        const auto action = tui.process_input(sources_.size());

        if (action.kind != InspectorTui::ActionKind::None) {
          switch (action.kind) {
          case InspectorTui::ActionKind::Quit:
            running = false;
            break;

          case InspectorTui::ActionKind::NextSource:
            active = (active + 1) % sources_.size();
            tui.reset_selection();
            force_poll = true;
            break;

          case InspectorTui::ActionKind::PreviousSource:
            active = (active + sources_.size() - 1) % sources_.size();
            tui.reset_selection();
            force_poll = true;
            break;

          case InspectorTui::ActionKind::SelectSource:
            if (action.source_index < sources_.size()) {
              active = action.source_index;
              tui.reset_selection();
              force_poll = true;
            }
            break;

          case InspectorTui::ActionKind::ToggleHistory:
            options.history = !options.history;
            force_poll = true;
            break;

          case InspectorTui::ActionKind::Redraw:
            break;

          case InspectorTui::ActionKind::Reconnect:
            reconnect_source(active, options.history, snapshots, has_snapshot);

            tui.reset_selection();

            force_poll = false;

            next_poll = std::chrono::steady_clock::now() + period;

            break;

          case InspectorTui::ActionKind::None:
            break;
          }

          break;
        }

        if (!tui.paused() && std::chrono::steady_clock::now() >= next_poll) {
          break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }

    return 0;
  }

private:
  struct Source {
    std::string label;
    std::unique_ptr<IInspectorSource> inspector;
  };

  void poll_source(std::size_t index, bool history,
                   std::vector<InspectorSnapshot> &snapshots,
                   std::vector<bool> &has_snapshot) {
    try {
      snapshots[index] = sources_[index].inspector->poll(history);
      snapshots[index].error.clear();
    } catch (const std::exception &e) {
      InspectorSnapshot failed;
      failed.shm_name = std::string(sources_[index].inspector->shm_name());
      failed.history = history;
      failed.error = e.what();
      snapshots[index] = std::move(failed);
    }

    has_snapshot[index] = true;
  }

  void reconnect_source(std::size_t index, bool history,
                        std::vector<InspectorSnapshot> &snapshots,
                        std::vector<bool> &has_snapshot) {
    try {
      sources_[index].inspector->reconnect();
      snapshots[index] = sources_[index].inspector->poll(history);
      snapshots[index].error.clear();
      has_snapshot[index] = true;
    } catch (const std::exception &e) {
      InspectorSnapshot failed;
      failed.shm_name = std::string(sources_[index].inspector->shm_name());
      failed.history = history;
      failed.error = e.what();
      snapshots[index] = std::move(failed);
      has_snapshot[index] = true;
    }
  }

  std::vector<Source> sources_;
};