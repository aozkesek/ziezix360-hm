package com.ziezix360.hm.patient;

import com.ziezix360.hm.patient.dao.model.Patient;
import nz.net.ultraq.thymeleaf.LayoutDialect;
import org.hibernate.SessionFactory;
import org.hibernate.boot.Metadata;
import org.hibernate.boot.MetadataSources;
import org.hibernate.boot.SessionFactoryBuilder;
import org.hibernate.boot.registry.StandardServiceRegistry;
import org.hibernate.boot.registry.StandardServiceRegistryBuilder;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.servlet.ViewResolver;
import org.springframework.web.servlet.config.annotation.EnableWebMvc;
import org.springframework.web.servlet.config.annotation.ResourceHandlerRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;
import org.thymeleaf.extras.springsecurity5.dialect.SpringSecurityDialect;
import org.thymeleaf.spring5.SpringTemplateEngine;
import org.thymeleaf.spring5.templateresolver.SpringResourceTemplateResolver;
import org.thymeleaf.spring5.view.ThymeleafViewResolver;
import org.thymeleaf.templatemode.TemplateMode;

import javax.annotation.PostConstruct;

@Configuration
@EnableWebMvc
public class PatientWebConfig implements WebMvcConfigurer {

    @Autowired
    ApplicationContext applicationContext;

    @Value("${userdb.name}")
    String dbName = "hm";
    @Value("${userdb.user}")
    String dbUser = "";
    @Value("${userdb.pass}")
    String dbPassword = "";
    @Value("${userdb.cstr}")
    String dbConnectionString = "";

    SessionFactory sessionFactory;

    @Override
    public void addResourceHandlers(ResourceHandlerRegistry registry) {
        registry.addResourceHandler("/static/**").addResourceLocations("classpath:WEB-INF/static/")
                .setCachePeriod(31556926);
    }

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
        mds.addAnnotatedClass(Patient.class);

        Metadata md = mds.buildMetadata();

        SessionFactoryBuilder sfb = md.getSessionFactoryBuilder();

        sessionFactory = sfb.build();

    }

    @Bean
    public SessionFactory sessionFactory() {
        return sessionFactory;
    }

    @Bean
    public SpringResourceTemplateResolver templateResolver(){
        SpringResourceTemplateResolver templateResolver = new SpringResourceTemplateResolver();
        templateResolver.setApplicationContext(this.applicationContext);
        templateResolver.setPrefix("classpath:WEB-INF/templates");
        templateResolver.setSuffix(".html");
        templateResolver.setTemplateMode(TemplateMode.HTML);
        templateResolver.setCacheable(true);
        return templateResolver;
    }

    @Bean
    public SpringTemplateEngine templateEngine(){
        SpringTemplateEngine templateEngine = new SpringTemplateEngine();
        templateEngine.setTemplateResolver(templateResolver());
        templateEngine.setEnableSpringELCompiler(true);
        templateEngine.addDialect(new LayoutDialect());
        templateEngine.addDialect(new SpringSecurityDialect());
        return templateEngine;
    }

    @Bean
    public ViewResolver viewResolver(){
        ThymeleafViewResolver viewResolver = new ThymeleafViewResolver();
        viewResolver.setTemplateEngine(templateEngine());
        viewResolver.setOrder(1);
        viewResolver.setViewNames(new String[] {".html", ".xhtml"});
        return viewResolver;
    }

}
