-- ---------------------------------------------------------------------------
-- Base de données du projet CORBA - Banque
-- Exécution :  mysql -u root -p < sql/schema.sql
-- ---------------------------------------------------------------------------

DROP DATABASE IF EXISTS banque;
CREATE DATABASE banque CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE banque;

CREATE TABLE etudiant (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(25) NOT NULL,
    createAt TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

INSERT INTO etudiant(name) VALUES ("Tendry"), ("Faniry"), ("Faneva");