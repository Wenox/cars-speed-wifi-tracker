package com.wenox.traffic.controller;

import com.wenox.traffic.domain.Info;
import com.wenox.traffic.service.InfoService;
import java.net.URI;
import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/info")
@RequiredArgsConstructor
public class InfoController {

  private final InfoService infoService;

  @PostMapping
  public ResponseEntity<Info> add(@RequestBody String message) {
    Info saved = infoService.add(message);

    return ResponseEntity
        .created(URI.create("/api/info/" + saved.getId()))
        .body(saved);
  }

  @GetMapping("/next")
  public String getNextMessage() {
    return infoService.getNextMessage();
  }
}
