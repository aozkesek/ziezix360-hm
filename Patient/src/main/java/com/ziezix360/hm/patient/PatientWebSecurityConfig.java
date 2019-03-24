package com.ziezix360.hm.patient;

import org.springframework.context.annotation.Configuration;
import org.springframework.security.config.annotation.authentication.builders.AuthenticationManagerBuilder;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.config.annotation.web.configuration.WebSecurityConfigurerAdapter;

@Configuration
@EnableWebSecurity
public class PatientWebSecurityConfig extends WebSecurityConfigurerAdapter {

    protected void configure(HttpSecurity httpSecurity) throws Exception {

        httpSecurity
                .authorizeRequests()
                    .antMatchers("/static/**", "/favicon.ico",
                            "/loggedout", "/signin", "/signin-error")
                    .permitAll()
                    .anyRequest().authenticated()

                .and().formLogin()
                    .loginPage("/signin")
                    .failureForwardUrl("/signin-error")
                    .successForwardUrl("/signedin")
                    .permitAll()

                .and().logout()
                    .logoutSuccessUrl("/loggedout")
                    .permitAll()
                    .invalidateHttpSession(true)
                    .clearAuthentication(true)

                .and().csrf()

                .and().cors();

    }


}
