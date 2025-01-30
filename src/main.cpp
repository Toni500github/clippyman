#include "EventData.hpp"
#include <chrono>
#include <cstdlib>
#include <clipboard/x11/ClipboardListenerX11.hpp>
#include <fmt/base.h>
#include <thread>

void CopyCallback(const CopyEvent &event) {
    fmt::println("Copy! {}", event.content);
}

int main() {
    CClipboardListenerX11 clipboardListener;

    clipboardListener.AddCopyCallback(CopyCallback);

    while (true) {
        fmt::println("Poll!");
        clipboardListener.PollClipboard();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}
