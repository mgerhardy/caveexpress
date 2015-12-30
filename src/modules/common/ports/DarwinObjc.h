#pragma once

#include <string>

void darwinInit ();
char* darwinGetHomeDirectory (const std::string& app);
void darwinRequestUserAttention (bool critical);
