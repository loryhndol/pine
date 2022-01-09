/**
 * @file Poller.cpp
 * @author 冯岳松 (yuesong-feng@foxmail.com)
 * @brief
 * @version 0.1
 * @date 2022-01-04
 *
 * @copyright Copyright (冯岳松) 2022
 *
 */
#include "Poller.h"

#include <unistd.h>

#include <cstring>

#include "Channel.h"
#include "util.h"


#ifdef OS_LINUX

#define MAX_EVENTS 1000
Poller::Poller() {
  fd_ = epoll_create1(0);
  ErrorIf(fd_ == -1, "epoll create error");
  events_ = new epoll_event[MAX_EVENTS];
  memset(events_, 0, sizeof(*events_) * MAX_EVENTS);
}

Poller::~Poller() {
  if (fd_ != -1) {
    close(fd_);
  }
  delete[] events_;
}

std::vector<Channel *> Poller::Poll(int timeout) {
  std::vector<Channel *> active_channels;
  int nfds = epoll_wait(fd_, events_, MAX_EVENTS, timeout);
  ErrorIf(nfds == -1, "epoll wait error");
  for (int i = 0; i < nfds; ++i) {
    Channel *ch = (Channel *)events_[i].data.ptr;
    ch->SetReadyEvents(events_[i].events);
    active_channels.push_back(ch);
  }
  return active_channels;
}

void Poller::UpdateChannel(Channel *ch) {
  int fd = ch->GetFd();
  struct epoll_event ev {};
  ev.data.ptr = ch;
  if (ch->GetListenEvents() & ch->kReadEvent) {
    ev.events |= EPOLLIN | EPOLLPRI;
  }
  if(ch->GetListenEvents() & ch->kWriteEvent{
    ev.events |= EPOLLOUT;
  }
  if(ch->GetListenEvents() & ch->kET){
    ev.events |= EPOLLET;
  }
  if (!ch->GetInEpoll()) {
    ErrorIf(epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
    ch->SetInEpoll();
  } else {
    ErrorIf(epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
  }
}

void Poller::DeleteChannel(Channel *ch) {
  int fd = ch->GetFd();
  ErrorIf(epoll_ctl(fd_, EPOLL_CTL_DEL, fd, nullptr) == -1, "epoll delete error");
  ch->SetInEpoll(false);
}
#endif

#ifdef OS_MACOS

Poller::Poller() {
  fd_ = kqueue();
  ErrorIf(fd_ == -1, "kqueue create error");
}

Poller::~Poller() {
  if (fd_ != -1) {
    close(fd_);
  }
}

std::vector<Channel *> Poller::Poll(int timeout) {

}

void Poller::UpdateChannel(Channel *ch) {

}

void Poller::DeleteChannel(Channel *ch) {
  
}
#endif