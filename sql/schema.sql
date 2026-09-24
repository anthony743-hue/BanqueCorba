-- ---------------------------------------------------------------------------
-- Base de données du projet CORBA - Banque
-- Exécution :  mysql -u root -p < sql/schema.sql
-- ---------------------------------------------------------------------------

DROP DATABASE IF EXISTS banque;
CREATE DATABASE banque CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE banque;

CREATE TABLE compte (
    id         INT AUTO_INCREMENT PRIMARY KEY,
    titulaire  VARCHAR(100)   NOT NULL,
    solde      DECIMAL(15,2)  NOT NULL DEFAULT 0.00,
    cree_le    TIMESTAMP      NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;          -- InnoDB obligatoire : on utilise les transactions

CREATE TABLE operation (
    id             INT AUTO_INCREMENT PRIMARY KEY,
    compte_id      INT            NOT NULL,
    type_operation VARCHAR(20)    NOT NULL,   -- DEPOT / RETRAIT / VIREMENT_*
    montant        DECIMAL(15,2)  NOT NULL,
    date_operation TIMESTAMP      NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_operation_compte
        FOREIGN KEY (compte_id) REFERENCES compte(id) ON DELETE CASCADE
) ENGINE=InnoDB;

-- Utilisateur applicatif utilisé par le serveur C++
CREATE USER IF NOT EXISTS 'banque'@'localhost' IDENTIFIED BY 'banque';
GRANT ALL PRIVILEGES ON banque.* TO 'banque'@'localhost';
FLUSH PRIVILEGES;

-- Jeu d'essai
INSERT INTO compte (titulaire, solde) VALUES
    ('Rakoto Jean',    150000.00),
    ('Rasoa Miora',     42000.00),
    ('Andry Herilala',    500.00);
