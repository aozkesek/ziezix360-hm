package com.ziezix360.hm.user.dao.impl;

import com.ziezix360hm.DaoImpl;
import com.ziezix360.hm.user.dao.intf.UserDao;
import com.ziezix360.hm.user.dao.model.User;
import org.springframework.stereotype.Component;

import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.Predicate;
import javax.persistence.criteria.Root;


@Component
public class UserDaoImpl extends DaoImpl<User> implements UserDao {
    private static final String GetByName = "GETBYNAME";
    private static final String Authenticate = "AUTHENTICATE";

    @Override
    public Class getModelClass() {
        return User.class;
    }

    @Override
    public Predicate buildCriteriaQuery(
            CriteriaBuilder criteriaBuilder, Root<User> root, String name, User model) {

        if (name == null)
            // this mean return ALL
            return root.isNotNull();

        switch (name) {
            case GetByName:
                return criteriaBuilder.equal(root.get("userName"), model.getUserName());

            case Authenticate:
                return criteriaBuilder.and(
                        criteriaBuilder.equal(root.get("userName"), model.getUserName()),
                        criteriaBuilder.equal(root.get("hashedSecret"), model.getHashedSecret()));

        }

        return root.isNull();
    }

    @Override
    public User getByUserName(String userName) {
        return query(GetByName, new User(userName)).get(0);
    }

    @Override
    public boolean authenticateUser(String userName, String hashedSecret) {
        User user = new User(userName);
        user.setHashedSecret(hashedSecret);
        user = query(Authenticate, user).get(0);
        if (user != null)
            return true;
        return false;

    }
}
