/*
 * This Java source file was generated by the Gradle 'init' task.
 */
package com.ziezix360.hm.user;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.ComponentScan;


@SpringBootApplication
@ComponentScan("com.ziezix360.hm.user")
public class UserApp {

    public static ApplicationContext context;

    public static void main(String[] args) {

        context = SpringApplication.run(UserApp.class, args);

    }

}
