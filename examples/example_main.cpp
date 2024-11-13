#include "MiniKDENotify/MiniKDENotify.hpp"
#include "MiniKDENotify/version.hpp"
#include <iostream>

int main() {
    try {
        auto notification = MiniKdeNotify::Notification::Builder().summary("Hello KDE").body(
                "This is a notification (no shit)").build();

        auto advanced = MiniKdeNotify::Notification::Builder("MyApp")
                .summary("Complex Notification")
                .body("This is a notification with actions and hints")
                .icon("dialog-warning")
                .timeout(10000)  // 10 seconds
                .addAction("action1", "Click Me")
                .addAction("action2", "Cancel")
                .addHint("sound-file", "/usr/share/sounds/notification.wav")
                .urgency(2)  // High urgency
                .build();

        notification->send();
        advanced->send();
    } catch (const MiniKdeNotify::NotificationError &e) {
        std::cerr << "Notification error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}