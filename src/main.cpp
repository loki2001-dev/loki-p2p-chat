
#include "PeerConnection.h"
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
    // 메인 로거 설정
    auto main_logger = spdlog::stdout_color_mt("MAIN");
    main_logger->set_pattern("[%H:%M:%S.%e] [%n] %v");

    main_logger->info("========================================");
    main_logger->info("      loki-p2p-char demo starting     ");
    main_logger->info("========================================");

    // INIT
    std::atomic<bool> pc1_gathering_done{false};
    std::atomic<bool> pc2_gathering_done{false};
    std::atomic<bool> offer_answer_exchanged{false};
    std::atomic<bool> pc1_connected{false};
    std::atomic<bool> pc2_connected{false};

    // ROLE
    PeerConnection pc1(true, "PC1");
    PeerConnection pc2(false, "PC2");

    // STATE
    pc1.onStateChange([&pc1_connected, &main_logger](juice_state state){
        if (state == JUICE_STATE_COMPLETED || state == JUICE_STATE_CONNECTED) {
            pc1_connected = true;
        }
    });

    pc2.onStateChange([&pc2_connected, &main_logger](juice_state state){
        if (state == JUICE_STATE_COMPLETED || state == JUICE_STATE_CONNECTED) {
            pc2_connected = true;
        }
    });

    // RECEIVE
    pc1.onMessage([&main_logger](const std::string& msg){
        main_logger->info("PC1 received: \"{}\"", msg);
    });
    pc2.onMessage([&main_logger](const std::string& msg){
        main_logger->info("PC2 received: \"{}\"", msg);
    });

    // CANDIDATES EXCHANGE
    pc1.onCandidate([&pc2, &main_logger](const std::string& cand){
        pc2.addRemoteCandidate(cand);
    });
    pc2.onCandidate([&pc1, &main_logger](const std::string& cand){
        pc1.addRemoteCandidate(cand);
    });

    // GATHERING DONE
    pc1.onGatheringDone([&pc1_gathering_done, &main_logger](){
        pc1_gathering_done = true;
    });
    pc2.onGatheringDone([&pc2_gathering_done, &main_logger](){
        pc2_gathering_done = true;
    });

    // GATHERING
    main_logger->info("");
    main_logger->info("Phase 1: ICE Candidate Gathering");
    main_logger->info("--------------------------------");

    pc1.startGathering();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pc2.startGathering();

    auto gathering_start = std::chrono::steady_clock::now();
    while (!pc1_gathering_done || !pc2_gathering_done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        auto elapsed = std::chrono::steady_clock::now() - gathering_start;
        if (elapsed > std::chrono::seconds(10)) {
            main_logger->error("ICE gathering timeout");
            return 1;
        }
    }

    auto gathering_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - gathering_start);
    main_logger->info("ICE gathering completed in {}ms", gathering_duration.count());

    // SDP EXCHANGE
    main_logger->info("");
    main_logger->info("Phase 2: SDP Offer/Answer Exchange");
    main_logger->info("----------------------------------");
    if (!offer_answer_exchanged) {
        offer_answer_exchanged = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::string offer = pc1.createOffer();
        if (!offer.empty()) {
            const auto& result = pc2.setRemoteDescription(offer);
            if (result) {
                std::string answer = pc2.createAnswer();
                if (!answer.empty()) {
                    const auto& result2 = pc1.setRemoteDescription(answer);
                    if (result2) {
                        // remote gathering done 설정
                        pc1.setRemoteGatheringDone();
                        pc2.setRemoteGatheringDone();
                    }
                } else {
                    return 1;
                }
            }
        } else {
            return 1;
        }
    }

    // ESTABLISHMENT
    main_logger->info("");
    main_logger->info("Phase 3: Connection Establishment");
    main_logger->info("---------------------------------");
    main_logger->info("Waiting for P2P connection to establish...");

    auto connection_start = std::chrono::steady_clock::now();
    int timeout = 50; // 5[SEC]
    while(timeout > 0 && (!pc1_connected || !pc2_connected)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        timeout--;
        if (timeout % 10 == 0) {
            main_logger->info("Still connecting... PC1: {} | PC2: {}",
                              juice_state_to_string(pc1.getState()),
                              juice_state_to_string(pc2.getState()));
        }
    }

    auto connection_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - connection_start);
    if (timeout <= 0) {
        return 1;
    }

    main_logger->info("P2P connection established successfully in {}ms", connection_duration.count());
    main_logger->info("");
    main_logger->info("Phase 4: Message Exchange Test");
    main_logger->info("------------------------------");

    // SEND
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (pc1_connected) {
        pc1.sendMessage("Hey PC2, Are you there ???");
    }
    if (pc2_connected) {
        pc2.sendMessage("Yes, PC1! Ready to chat !!!");
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // FINALIZE
    main_logger->info("");
    main_logger->info("========================================");
    main_logger->info("     loki-p2p-chat demo successfully       ");
    main_logger->info("========================================");
    main_logger->info("Final Statistics:");
    main_logger->info(" - ICE Gathering Time: {}ms", gathering_duration.count());
    main_logger->info(" - Connection Time: {}ms", connection_duration.count());
    main_logger->info(" - PC1 Final State: {}", juice_state_to_string(pc1.getState()));
    main_logger->info(" - PC2 Final State: {}", juice_state_to_string(pc2.getState()));
    main_logger->info("");

    return 0;
}