// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#define DefineTopicTrait(Namespace, TopicName) template<> struct TopicTrait<Namespace::TopicName> { using PubSubType = Namespace::TopicName##PubSubType; static constexpr const char* DefaultName() { return #TopicName; }; static constexpr const char* TypeName() { return #Namespace "::" #TopicName; } }
