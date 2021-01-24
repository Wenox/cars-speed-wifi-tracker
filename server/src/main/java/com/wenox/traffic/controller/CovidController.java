package com.wenox.traffic.controller;

import com.wenox.traffic.service.CovidService;
import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;

@RestController
@RequestMapping("/api/covid-cases")
@RequiredArgsConstructor
public class CovidController {

  private final CovidService covidService;

  @GetMapping
  public ResponseEntity<String> getDailyNewCases(RestTemplate restTemplate) {
    return ResponseEntity.ok(covidService.getDailyCases(restTemplate));
  }
}
