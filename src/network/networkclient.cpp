
#include "networkclient.h"
std::vector<CURL*> NetworkClient::curls = {};
StringMap NetworkClient::defaultHeaders = {{"user-agent","Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36"}};
std::string NetworkClient::defaultUA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36";
bool NetworkClient::initialised = false;

