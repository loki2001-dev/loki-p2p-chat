#pragma once

#include <string>
#include <functional>
#include <juice/juice.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class PeerConnection {
public:
    PeerConnection(bool is_controlling = true, const std::string& name = "PeerConnection");
    ~PeerConnection();

    bool startGathering();
    std::string createOffer();
    std::string createAnswer();
    bool setRemoteDescription(const std::string& sdp);
    void setRemoteGatheringDone();
    bool addRemoteCandidate(const std::string& candidate);

    void sendMessage(const std::string& msg);

    juice_state getState() const;

    void onMessage(std::function<void(const std::string&)> cb);
    void onStateChange(std::function<void(juice_state)> cb);
    void onCandidate(std::function<void(const std::string&)> cb);
    void onGatheringDone(std::function<void()> cb);

private:
    juice_agent* _agent = nullptr;
    bool _is_controlling;
    bool _message_sent = false;
    std::string _name;
    std::shared_ptr<spdlog::logger> _logger;

    std::function<void(const std::string&)> _msg_cb;
    std::function<void(juice_state)> _state_cb;
    std::function<void(const std::string&)> _candidate_cb;
    std::function<void()> _gathering_done_cb;

    static void on_data_cb(juice_agent* agent, const char* data, size_t size, void* user_ptr);
    static void on_state_cb(juice_agent* agent, juice_state state, void* user_ptr);
    static void on_candidate_cb(juice_agent* agent, const char* sdp, void* user_ptr);
    static void on_gathering_done_cb(juice_agent* agent, void* user_ptr);
};