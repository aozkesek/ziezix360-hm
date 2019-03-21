package com.ziezix360.hm.user.controller;

import com.ziezix360.hm.user.dao.model.User;

public class UserRequest {
    User user;
    String method;

    public User getUser() {
        return user;
    }

    public void setUser(User user) {
        this.user = user;
    }

    public String getMethod() {
        return method;
    }

    public void setMethod(String method) {
        this.method = method;
    }
}
