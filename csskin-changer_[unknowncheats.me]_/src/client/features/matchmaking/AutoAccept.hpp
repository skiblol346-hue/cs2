#pragma once

class AutoAccept {
public:
    void initialize();
    void reset();
    void process_queue();
    void on_match_found();
    void on_panorama_event(const char* event_name);
    void queue_if_match_ready(const char* text);
    const char* resolve_panorama_text(const char* text);
};
