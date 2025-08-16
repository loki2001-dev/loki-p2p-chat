#include "PeerConnection.h"
#include <spdlog/fmt/fmt.h>

PeerConnection::PeerConnection(bool is_controlling, const std::string& name)
    : _is_controlling(is_controlling),
    _message_sent(false),
    _name(name) {

    _logger = spdlog::stdout_color_mt(_name);
    _logger->set_pattern("[%H:%M:%S.%e] [%n] %v");

    juice_config cfg{};
    cfg.concurrency_mode = JUICE_CONCURRENCY_MODE_THREAD;
    cfg.local_port_range_begin = 0;
    cfg.local_port_range_end = 0;
    cfg.stun_server_host = "stun.l.google.com";
    cfg.stun_server_port = 19302;
    cfg.cb_recv = PeerConnection::on_data_cb;
    cfg.cb_state_changed = PeerConnection::on_state_cb;
    cfg.cb_candidate = PeerConnection::on_candidate_cb;
    cfg.cb_gathering_done = PeerConnection::on_gathering_done_cb;
    cfg.user_ptr = this;

    juice_set_log_level(JUICE_LOG_LEVEL_ERROR);
    _agent = juice_create(&cfg);
    if (_agent) {
        _logger->info("Agent created successfully - STUN: {}:{}", cfg.stun_server_host, cfg.stun_server_port);
    }
}

PeerConnection::~PeerConnection() {
    _logger->info("Destroying connection");
    if (_agent) {
        juice_destroy(_agent);
        _logger->info("Agent destroyed successfully");
    }
}

bool PeerConnection::startGathering() {
    if (!_agent) {
        _logger->error("Cannot start gathering: agent is null");
        return false;
    }

    const auto& success = juice_gather_candidates(_agent) == JUICE_ERR_SUCCESS;
    return success;
}

std::string PeerConnection::createOffer() {
    if (!_agent) {
        _logger->error("Cannot create offer: agent is null");
        return "";
    }

    char buffer[4096] = {0x00, };
    if (juice_get_local_description(_agent, buffer, sizeof(buffer)) == JUICE_ERR_SUCCESS) {
        std::string offer(buffer);
        return offer;
    }
    return "";
}

std::string PeerConnection::createAnswer() {
    std::string answer = createOffer();
    if (!answer.empty()) {
        return answer;
    }
    return "";
}

bool PeerConnection::setRemoteDescription(const std::string& sdp) {
    if (!_agent) {
        _logger->error("Cannot set remote description: agent is null");
        return false;
    }

    const auto& success = juice_set_remote_description(_agent, sdp.c_str()) == JUICE_ERR_SUCCESS;
    if (success) {
        return success;
    }
    return false;
}

void PeerConnection::setRemoteGatheringDone() {
    if (_agent) {
        juice_set_remote_gathering_done(_agent);
    }
}

bool PeerConnection::addRemoteCandidate(const std::string& candidate) {
    if (!_agent) {
        return false;
    }

    const auto& success = juice_add_remote_candidate(_agent, candidate.c_str()) == JUICE_ERR_SUCCESS;
    if (success) {
        return success;
    }
    return false;
}

void PeerConnection::sendMessage(const std::string& msg) {
    if (!_agent) {
        return;
    }

    if (_message_sent) {
        return;
    }

    juice_state state = juice_get_state(_agent);
    if (state != JUICE_STATE_CONNECTED && state != JUICE_STATE_COMPLETED) {
        return;
    }

    _message_sent = true;

    const auto& result = juice_send(_agent, msg.c_str(), msg.size());
    if (result != JUICE_ERR_SUCCESS) {
        _message_sent = false;
    }
}

juice_state PeerConnection::getState() const {
    if (!_agent) {
        return JUICE_STATE_FAILED;
    }
    return juice_get_state(_agent);
}

void PeerConnection::onMessage(std::function<void(const std::string&)> cb) {
    _msg_cb = cb;

}
void PeerConnection::onStateChange(std::function<void(juice_state)> cb) {
    _state_cb = cb;
}

void PeerConnection::onCandidate(std::function<void(const std::string&)> cb) {
    _candidate_cb = cb;
}

void PeerConnection::onGatheringDone(std::function<void()> cb) {
    _gathering_done_cb = cb;
}

void PeerConnection::on_data_cb(juice_agent* agent, const char* data, size_t size, void* user_ptr) {
    auto* self = static_cast<PeerConnection*>(user_ptr);
    std::string msg(data, size);

    if (self->_msg_cb) {
        self->_msg_cb(msg);
    }
}

void PeerConnection::on_state_cb(juice_agent* agent, juice_state state, void* user_ptr) {
    auto* self = static_cast<PeerConnection*>(user_ptr);
    std::string state_name = juice_state_to_string(state);

    if (self->_state_cb) {
        self->_logger->info("State changed to: {}", state_name);
        self->_state_cb(state);
    }
}

void PeerConnection::on_candidate_cb(juice_agent* agent, const char* sdp, void* user_ptr) {
    auto* self = static_cast<PeerConnection*>(user_ptr);
    if (self->_candidate_cb) {
        std::string candidate(sdp);
        self->_candidate_cb(candidate);
    }
}

void PeerConnection::on_gathering_done_cb(juice_agent* agent, void* user_ptr) {
    auto* self = static_cast<PeerConnection*>(user_ptr);
    if (self->_gathering_done_cb) {
        self->_gathering_done_cb();
    }
}