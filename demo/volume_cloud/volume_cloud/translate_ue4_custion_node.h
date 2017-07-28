#pragma once
#include <fstream>
#include <string>
#include <iostream>
bool translate_ue4_node(const std::string& file_name, const std::string& main_function, const std::string& output = "custon_node_output.temporary.txt");