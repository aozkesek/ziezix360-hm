package com.ziezix360hm;

import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.Predicate;
import javax.persistence.criteria.Root;

public class DaoTestImpl extends DaoImpl<ModelTest> {

    @Override
    public Class getModelClass() {
        return ModelTest.class;
    }

    @Override
    public Predicate buildCriteriaQuery(CriteriaBuilder criteriaBuilder,
                                        Root<ModelTest> root, String name,
                                        ModelTest model) {
        return null;
    }
}
