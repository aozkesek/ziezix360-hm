package com.ziezix360hm;

import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.Predicate;
import javax.persistence.criteria.Root;
import java.util.List;

public interface Dao<T extends DaoModel> {

    T create(T model);
    T update(T model);
    T read(T model);
    T delete(T model);

    List<T> list();
    List<T> query(String name, T model);

    Predicate buildCriteriaQuery(CriteriaBuilder criteriaBuilder,
                                 Root<T> root, String name, T model);
    Class getModelClass();
}
