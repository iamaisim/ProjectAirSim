// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/topic.hpp"

#include <memory>
#include <string>

#include "component.hpp"
#include "constant.hpp"
#include "core_sim/error.hpp"
#include "core_sim/logger.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

class Topic::Impl {
 public:
  Impl(const std::string& name, const std::string& path, TopicType type,
       int frequency, MessageType message_type);

  const std::string& GetName();

  const std::string& GetPath();

  TopicType GetType();

  int GetFrequency();

  MessageType GetMessageType();

  bool IsEmpty() const;

  void SetFrequency(int value);

 private:
  std::string name_;
  std::string path_;
  TopicType type_;
  int frequency_;
  MessageType msg_type_;
};

// class Topic

Topic::Topic() : pimpl_(std::shared_ptr<Topic::Impl>(nullptr)) {}

Topic::Topic(const std::string& name, const std::string& path, TopicType type,
             int frequency, MessageType message_type)
    : pimpl_(std::shared_ptr<Topic::Impl>(
          new Topic::Impl(name, path, type, frequency, message_type))) {}

const std::string& Topic::GetName() const { return pimpl_->GetName(); }

const std::string& Topic::GetPath() const { return pimpl_->GetPath(); }

TopicType Topic::GetType() const { return pimpl_->GetType(); }

int Topic::GetFrequency() const { return pimpl_->GetFrequency(); }

MessageType Topic::GetMessageType() const { return pimpl_->GetMessageType(); }

bool Topic::IsEmpty() const { return pimpl_->IsEmpty(); }

void Topic::SetFrequency(int value) { return pimpl_->SetFrequency(value); }

// -----------------------------------------------------------------------------------------------
// class Topic::Impl

Topic::Impl::Impl(const std::string& name, const std::string& path,
                  TopicType type, int frequency, MessageType message_type)
    : name_(name),
      path_(path + "/" + name),
      type_(type),
      frequency_(frequency),
      msg_type_(message_type) {}

const std::string& Topic::Impl::GetName() { return name_; }

const std::string& Topic::Impl::GetPath() { return path_; }

TopicType Topic::Impl::GetType() { return type_; }

int Topic::Impl::GetFrequency() { return frequency_; }

MessageType Topic::Impl::GetMessageType() { return msg_type_; }

bool Topic::Impl::IsEmpty() const { return path_.empty(); }

void Topic::Impl::SetFrequency(int value) { frequency_ = value; }

}  // namespace projectairsim
}  // namespace microsoft
