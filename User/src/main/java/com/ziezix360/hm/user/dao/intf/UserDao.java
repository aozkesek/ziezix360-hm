package com.ziezix360.hm.user.dao.intf;

import com.ziezix360.hm.user.dao.model.User;
import com.ziezix360hm.Dao;

public interface UserDao extends Dao<User> {

    User getByUserName(String userName);
    boolean authenticateUser(String userName, String hashedSecret);

}
