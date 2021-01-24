package com.wenox.traffic.service;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

@Service
public class CovidService {

  private static final Pattern re = Pattern.compile("kraj;(\\d+?);");
  private String cached;
  private boolean isCached = false;

  public String getDailyCases(RestTemplate restTemplate) {
    if (!isCached || "ERROR".equals(cached)) {
      String response = restTemplate
          .getForObject("https://www.gov.pl/web/koronawirus/wykaz-zarazen-koronawirusem-sars-cov-2", String.class);
      assert response != null;
      Matcher match = re.matcher(response);
      if (match.find()) {
        cached = match.group(1);
      } else {
        cached = "ERROR";
      }
      isCached = true;
    }
    return cached;
  }

}
