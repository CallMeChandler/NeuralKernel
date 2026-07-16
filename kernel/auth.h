#pragma once
namespace auth { void initialize(); bool login(); void handle_input(char); bool authenticated(); const char* username(); void logout(); }
