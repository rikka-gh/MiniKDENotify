#pragma  once

//#define MINI_KDE_NOTIFY_DEBUG // enable debug logs

#include <dbus/dbus.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

namespace MiniKdeNotify {
    class NotificationError : public std::runtime_error {
    public:
        explicit NotificationError(const std::string &message)
                : std::runtime_error(message) {}
    };

    class Notification {
    public:
        struct Action {
            std::string key;
            std::string label;
        };

        using Hints = std::map<std::string, std::string>;

        class DBusConnector {
        public:
            DBusConnector() : conn(nullptr) {
                DBusError err;
                dbus_error_init(&err);

                conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
                if (dbus_error_is_set(&err)) {
                    std::string error_msg = "Connection Error (" + std::string(err.name) +
                                            "): " + std::string(err.message);
                    dbus_error_free(&err);
                    throw NotificationError(error_msg);
                }

                if (!conn) {
                    throw NotificationError("Failed to connect to the D-Bus session bus.");
                }

                dbus_bus_request_name(conn, "org.example.NotificationSender",
                                      DBUS_NAME_FLAG_REPLACE_EXISTING, &err);

                if (dbus_error_is_set(&err)) {
                    std::string error_msg = "Name Error (" + std::string(err.name) +
                                            "): " + std::string(err.message);
                    dbus_error_free(&err);
                    throw NotificationError(error_msg);
                }
            }

            ~DBusConnector() {
                if (conn) {
                    dbus_connection_unref(conn);
                }
            }

            DBusConnector(const DBusConnector &) = delete;

            DBusConnector &operator=(const DBusConnector &) = delete;

            DBusConnector(DBusConnector &&) = delete;

            DBusConnector &operator=(DBusConnector &&) = delete;

            explicit operator DBusConnection *() { return conn; }

            DBusConnection *get() { return conn; }

        private:
            DBusConnection *conn;
        };

        class Builder {
        public:
            explicit Builder(const std::string &app_name = "")
                    : notification(std::make_shared<Notification>(app_name)) {}

            Builder &summary(const std::string &summary) {
                notification->summary_ = summary;
                return *this;
            }

            Builder &body(const std::string &body) {
                notification->body_ = body;
                return *this;
            }

            Builder &icon(const std::string &icon) {
                notification->icon_ = icon;
                return *this;
            }

            Builder &timeout(int32_t timeout_ms) {
                notification->timeout_ = timeout_ms;
                return *this;
            }

            Builder &addAction(const std::string &key, const std::string &label) {
                notification->actions_.push_back({key, label});
                return *this;
            }

            Builder &addHint(const std::string &key, const std::string &value) {
                notification->hints_[key] = value;
                return *this;
            }

            Builder &urgency(uint8_t urgency) {
                notification->urgency_ = urgency;
                return *this;
            }

            std::shared_ptr<Notification> build() {
                return notification;
            }

        private:
            std::shared_ptr<Notification> notification;
        };

        explicit Notification(std::string app_name = "MinimalNotificationApp")
                : app_name_(std::move(app_name)), replaces_id_(0), icon_("dialog-information"), timeout_(5000),
                  urgency_(1) {}

        void send() {
            static DBusConnector conn;

            DBusMessage *msg = dbus_message_new_method_call(
                    "org.freedesktop.Notifications",
                    "/org/freedesktop/Notifications",
                    "org.freedesktop.Notifications",
                    "Notify");

            if (!msg) {
                throw NotificationError("Failed to create message for the notification.");
            }

            try {
                DBusMessageIter iter;
                dbus_message_iter_init_append(msg, &iter);

                appendBasicString(&iter, app_name_);
                appendBasicUint32(&iter, replaces_id_);
                appendBasicString(&iter, icon_);
                appendBasicString(&iter, summary_);
                appendBasicString(&iter, body_);

                appendActionsArray(&iter);

                appendHintsDict(&iter);

                dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &timeout_);

                DBusError err;
                dbus_error_init(&err);
                DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn.get(), msg, -1, &err);

                if (dbus_error_is_set(&err)) {
                    std::string error_msg = "DBus Error (" + std::string(err.name) +
                                            "): " + std::string(err.message);
                    dbus_error_free(&err);
                    throw NotificationError(error_msg);
                }

                if (reply) {
                    dbus_message_unref(reply);
                }
            } catch (...) {
                dbus_message_unref(msg);
                throw;
            }

            dbus_message_unref(msg);
        }

    private:
        std::string app_name_;
        uint32_t replaces_id_;
        std::string icon_;
        std::string summary_;
        std::string body_;
        std::vector<Action> actions_;
        Hints hints_;
        int32_t timeout_;
        uint8_t urgency_;

        static void appendBasicString(DBusMessageIter *iter, const std::string &str) {
            const char *cstr = str.c_str();
            dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &cstr);
        }

        static void appendBasicUint32(DBusMessageIter *iter, uint32_t value) {
            dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &value);
        }

        void appendActionsArray(DBusMessageIter *iter) {
            DBusMessageIter array_iter;
            dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "s", &array_iter);

            for (const auto &action: actions_) {
                const char *key = action.key.c_str();
                const char *label = action.label.c_str();
                dbus_message_iter_append_basic(&array_iter, DBUS_TYPE_STRING, &key);
                dbus_message_iter_append_basic(&array_iter, DBUS_TYPE_STRING, &label);
            }

            dbus_message_iter_close_container(iter, &array_iter);
        }

        void appendHintsDict(DBusMessageIter *iter) {
            DBusMessageIter array_iter;
            dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", &array_iter);

            if (urgency_ != 1) {
                DBusMessageIter dict_iter, variant_iter;
                const char *urgency_key = "urgency";
                dbus_message_iter_open_container(&array_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &dict_iter);
                dbus_message_iter_append_basic(&dict_iter, DBUS_TYPE_STRING, &urgency_key);
                dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_VARIANT, "y", &variant_iter);
                dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BYTE, &urgency_);
                dbus_message_iter_close_container(&dict_iter, &variant_iter);
                dbus_message_iter_close_container(&array_iter, &dict_iter);
            }

            for (const auto &hint: hints_) {
                DBusMessageIter dict_iter, variant_iter;
                const char *key = hint.first.c_str();
                const char *value = hint.second.c_str();

                dbus_message_iter_open_container(&array_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &dict_iter);
                dbus_message_iter_append_basic(&dict_iter, DBUS_TYPE_STRING, &key);
                dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
                dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_STRING, &value);
                dbus_message_iter_close_container(&dict_iter, &variant_iter);
                dbus_message_iter_close_container(&array_iter, &dict_iter);
            }

            dbus_message_iter_close_container(iter, &array_iter);
        }
    };
}