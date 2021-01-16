package com.wenox.traffic.controller;

import com.wenox.traffic.SpeedRepository;
import com.wenox.traffic.domain.Speed;
import java.net.URI;
import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/speed")
@RequiredArgsConstructor
public class SpeedController {

  private final SpeedRepository speedRepository;

  @PostMapping
  public ResponseEntity<Speed> add(@RequestBody String speed) {
    Speed saved = speedRepository.save(new Speed(Double.valueOf(speed)));

    return ResponseEntity
        .created(URI.create("/api/speed/" + saved.getId()))
        .body(saved);
  }

}
