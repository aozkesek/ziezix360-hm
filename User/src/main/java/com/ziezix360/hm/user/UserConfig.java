package com.ziezix360.hm.user;

import com.ziezix360.hm.user.dao.model.User;
import org.hibernate.SessionFactory;
import org.hibernate.boot.Metadata;
import org.hibernate.boot.MetadataSources;
import org.hibernate.boot.SessionFactoryBuilder;
import org.hibernate.boot.registry.StandardServiceRegistry;
import org.hibernate.boot.registry.StandardServiceRegistryBuilder;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import javax.annotation.PostConstruct;

@Configuration
public class UserConfig {

    @Value("${userdb.name}")
    String dbName = "hm";
    @Value("${userdb.user}")
    String dbUser = "";
    @Value("${userdb.pass}")
    String dbPassword = "";
    @Value("${userdb.cstr}")
    String dbConnectionString = "";

    SessionFactory sessionFactory;

    @PostConstruct
    public void postConstruct() {

        StandardServiceRegistryBuilder ssrb = new StandardServiceRegistryBuilder();
        ssrb.applySetting("hibernate.connection.driver_class", "org.hsqldb.jdbc.JDBCDriver")
                .applySetting("hibernate.connection.url", dbConnectionString + dbName)
                .applySetting("hibernate.connection.username", dbUser)
                .applySetting("hibernate.connection.password", dbPassword)

                .applySetting("hibernate.dialect", "org.hibernate.dialect.HSQLDialect")
                .applySetting("hibernate.current_session_context_class", "thread")

                .applySetting("hibernate.show_sql", "true")
                .applySetting("hibernate.format_sql", "true")

        ;

        StandardServiceRegistry ssr = ssrb.build();

        MetadataSources mds = new MetadataSources(ssr);
        mds.addAnnotatedClass(User.class);

        Metadata md = mds.buildMetadata();

        SessionFactoryBuilder sfb = md.getSessionFactoryBuilder();

        sessionFactory = sfb.build();

    }

    @Bean
    public SessionFactory sessionFactory() {
        return sessionFactory;
    }
}
