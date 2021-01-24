package com.wenox.traffic.controller;

import com.wenox.traffic.domain.Temperature;
import com.wenox.traffic.domain.TemperatureDto;
import com.wenox.traffic.repository.TemperatureRepository;
import java.net.URI;
import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/temperatures")
@RequiredArgsConstructor
public class TemperatureController {

  private final TemperatureRepository temperatureRepository;

  @PostMapping
  public ResponseEntity<Temperature> add(@RequestBody TemperatureDto dto) {
    Temperature saved = temperatureRepository.save(new Temperature(dto.getValue1(), dto.getValue2()));

    return ResponseEntity
        .created(URI.create("/api/temperatures/" + saved.getId()))
        .body(saved);

  }

}
