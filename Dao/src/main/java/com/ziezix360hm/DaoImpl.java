package com.ziezix360hm;

import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.Transaction;
import org.springframework.beans.factory.annotation.Autowired;

import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.CriteriaQuery;
import javax.persistence.criteria.Root;
import java.util.List;

public abstract class DaoImpl<T extends DaoModel> implements Dao<T> {

    enum ExecOperator {C, R, U, D, L, F}

    @Autowired
    private SessionFactory sessionFactory;

    public SessionFactory getSessionFactory() {
        return sessionFactory;
    }

    public void setSessionFactory(SessionFactory sessionFactory) {
        this.sessionFactory = sessionFactory;
    }

    protected T execute(ExecOperator operator, T model) {

        Session session = getSessionFactory().openSession();
        Transaction transaction = session.beginTransaction();

        try {

            switch (operator) {
                case C:
                    session.save(model);
                    break;

                case R:
                    model = (T)session.get(model.getClass(), model.getId());
                    break;

                case D:
                    session.delete(model);
                    break;

                case U:
                    session.update(model);
                    break;

                case F:
                    break;


            }

            transaction.commit();
            return model;

        } catch (Exception e) {
            transaction.rollback();
            return null;
        } finally {
            session.close();
        }

    }

    @Override
    public T create(T model) {
        return execute(ExecOperator.C, model);
    }

    @Override
    public T update(T model) {
        return execute(ExecOperator.U, model);
    }

    @Override
    public T read(T model) {
        return execute(ExecOperator.R, model);
    }

    @Override
    public T delete(T model) {
        return execute(ExecOperator.D, model);
    }

    @Override
    public List<T> query(String name, T model) {

        Session session = getSessionFactory().openSession();
        CriteriaBuilder criteriaBuilder = getSessionFactory().getCriteriaBuilder();
        CriteriaQuery<T> tCriteriaQuery = criteriaBuilder.createQuery(getModelClass());
        Root<T> tRoot = tCriteriaQuery.from(getModelClass());
        tCriteriaQuery.select(tRoot);

        tCriteriaQuery.where(
                buildCriteriaQuery(criteriaBuilder, tRoot, name, model)
        );

        Transaction transaction = session.beginTransaction();
        List<T> tList = session
                .createQuery( tCriteriaQuery )
                .list();
        transaction.commit();
        session.close();
        return tList;

    }

    @Override
    public List<T> list() {
        return query(null, null);
    }

}
