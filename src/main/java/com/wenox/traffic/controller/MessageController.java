package com.wenox.traffic.controller;

import com.wenox.traffic.domain.Message;
import com.wenox.traffic.service.MessageService;
import java.net.URI;
import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/messages")
@RequiredArgsConstructor
public class MessageController {

  private final MessageService messageService;

  @PostMapping
  public ResponseEntity<Message> add(@RequestBody String message) {
    Message saved = messageService.add(message);

    return ResponseEntity
        .created(URI.create("/api/messages/" + saved.getId()))
        .body(saved);
  }

  @GetMapping("/next")
  public String getNextMessage() {
    return messageService.getNextMessage();
  }
}
