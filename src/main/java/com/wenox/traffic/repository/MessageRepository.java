package com.wenox.traffic.repository;

import com.wenox.traffic.domain.Info;
import java.util.List;
import org.springframework.data.repository.CrudRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface MessageRepository extends CrudRepository<Info, Integer> {

  List<Info> findAll();
}
