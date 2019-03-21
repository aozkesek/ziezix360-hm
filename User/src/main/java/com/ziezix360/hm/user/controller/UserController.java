package com.ziezix360.hm.user.controller;

import com.ziezix360.hm.user.dao.intf.UserDao;
import com.ziezix360.hm.user.dao.model.User;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/user")
public class UserController {

    @Autowired
    UserDao userDao;

    @RequestMapping("/authenticate")
    @ResponseBody
    public UserResponse userAuthenticate(@RequestBody UserRequest request) {
        UserResponse response = new UserResponse();

        try {

            if (userDao.authenticateUser(request.user.getUserName(), request.user.getHashedSecret())) {
                response.setReturnCode(0);
                response.setResult(true);
                response.setReturnMessage("OK");
            } else {
                response.setReturnCode(500);
                response.setResult(false);
                response.setReturnMessage("NOT-AUTHENTICATED");
             }
        } catch (Exception ex) {
            response.setReturnCode(400);
            response.setResult(false);
            response.setReturnMessage("USER-NOT-FOUND");
        }

        return response;
    }

    @RequestMapping("/details")
    @ResponseBody
    public UserResponse userDetails(@RequestBody UserRequest request) {
        UserResponse response = new UserResponse();

        try {
            User user = userDao.getByUserName(request.user.getUserName());
            response.setUser(user);
            response.setReturnCode(0);
            response.setResult(true);
            response.setReturnMessage("OK");
            return response;
        } catch (Exception ex) {
            response.setReturnCode(400);
            response.setResult(false);
            response.setReturnMessage("USER-NOT-FOUND");
        }


        return response;
    }

    @RequestMapping("/list")
    @ResponseBody
    public UserResponse userList(@RequestBody UserRequest request) {
        UserResponse response = new UserResponse();

        response.setUsers(userDao.list());
        response.setReturnCode(0);
        response.setResult(true);
        response.setReturnMessage("OK");

        return response;
    }

}
