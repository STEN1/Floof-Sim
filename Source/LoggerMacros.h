//
// Created by Adrian Drevland on 08/09/2022.
//

#ifndef FLOOF_LOGGERMACROS_H
#define FLOOF_LOGGERMACROS_H
#include "Input.h"
#include "Logger.h"

#define LOG_WARNING(message) Input::s_Logger->log(message)
#define LOG_ERROR(message) Input::s_Logger->log(::Utils::Logger::LogType::ERROR,message);
#define LOG_CRITICAL(message) Input::s_Logger->log(::Utils::Logger::LogType::CRITICAL,message);

#endif //FLOOF_LOGGERMACROS_H