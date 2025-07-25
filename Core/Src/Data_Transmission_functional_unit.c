/*"
 * a.c
 *
 *  Created on: Jul 9, 2025
 *      Author: suneel.kataraki
 */
#include "Data_Transmission_functional_unit.h"
#include "uds_services.c"
#include"main.h"
#include "can_tp.h"



/****************************************************ReadDataByIdentifier*******************************************************/
uint8_t data_record_1[] = {0x12, 0x34, 0x54, 0x78, 0x98, 0x12, 0x56, 0x21, 0x98, 0x99, 0x66, 0x22};
uint8_t data_record_2[] = {0x56, 0x78};
uint8_t data_record_3[] = {0x9A, 0xBC};

/**
 * @brief Impl�mente le service ReadDataByIdentifier (0x22)
 * @param data : Pointeur vers les donn�es de la requ�te (contenant les dataIdentifiers)
 * @param data_length : Longueur des donn�es
 */
void uds_read_data_by_identifier(uint8_t* data, uint8_t data_length) {

	reset_s3_timer();
    // V�rification de la longueur minimale et modulo 2 de la requ�te
    if (data_length < 2 || data_length % 2 != 0) {
        send_negative_response_read_data_by_identifier(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // V�rification de la longueur maximale de la requ�te
    if (data_length > MAX_DATA_SIZE) {
        send_negative_response_read_data_by_identifier(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t response[MAX_DATA_SIZE] = {0};  // R�ponse maximale de 64 octets
    uint8_t response_index = 0;  // Index de remplissage de la r�ponse
    bool did_supported = false;  // Indicateur pour v�rifier si au moins un DID est support�

    response[response_index++] = UDS_READ_DATA_BY_IDENTIFIER + 0x40;  // SID de r�ponse 0x62

    // Boucle pour chaque dataIdentifier
    for (uint8_t i = 0; i < data_length; i += 2) {
        uint16_t did = (data[i] << 8) | data[i + 1];  // R�cup�ration du DID

        // V�rification si le service est support� pour chaque DID dans la session active
        if (!is_service_allowed(UDS_READ_DATA_BY_IDENTIFIER)) {
            send_negative_response_read_data_by_identifier(NRC_CONDITIONS_NOT_CORRECT);
            return;
        }

        // V�rification des conditions de s�curit� pour le DID
        if (is_security_required_for_did(did) && !uds_session.security_access_granted) {
            send_negative_response_read_data_by_identifier(NRC_SECURITY_ACCESS_DENIED);
            return;
        }

        switch (did) {
            case SUPPORTED_DID_1:
                // Ajout du DID et des donn�es associ�es � la r�ponse
                response[response_index++] = data[i];       // MSB du DID
                response[response_index++] = data[i + 1];   // LSB du DID
                memcpy(&response[response_index], data_record_1, sizeof(data_record_1));
                response_index += sizeof(data_record_1);
                did_supported = true;
                break;

            case SUPPORTED_DID_2:
                response[response_index++] = data[i];       // MSB du DID
                response[response_index++] = data[i + 1];   // LSB du DID
                memcpy(&response[response_index], data_record_2, sizeof(data_record_2));
                response_index += sizeof(data_record_2);
                did_supported = true;
                break;

            case SUPPORTED_DID_3:
                response[response_index++] = data[i];       // MSB du DID
                response[response_index++] = data[i + 1];   // LSB du DID
                memcpy(&response[response_index], data_record_3, sizeof(data_record_3));
                response_index += sizeof(data_record_3);
                did_supported = true;
                break;

            default:
                // Si le DID n'est pas support�, continuer la boucle sans r�pondre imm�diatement
                continue;
        }

        // V�rifier si la taille de la r�ponse d�passe la limite du protocole de transport
        if (response_index > MAX_DATA_SIZE) {
            send_negative_response_read_data_by_identifier(NRC_RESPONSE_TOO_LONG);
            return;
        }
    }


    // Si aucun DID n'est support�, renvoyer une r�ponse n�gative
    if (!did_supported) {
        send_negative_response_read_data_by_identifier(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    if(response_index >= 7)
    {
    	can_tp_send(0x7E0, response, response_index);
    }
    else
    {
    	send_can_message(response, response_index);
    }

    // Envoyer la r�ponse positive avec tous les DIDs trait�s

    // send_uart_message(response, response_index);
}

bool is_security_required_for_did(uint16_t did) {
    // Ajoutez ici la logique pour v�rifier si un DID n�cessite un acc�s de s�curit�
    return false;  // Par d�faut, supposons que les DIDs ne n�cessitent pas de s�curit�
}

/**
 * @brief Envoie une r�ponse positive pour le service ReadDataByIdentifier (0x22)
 * @param dataIdentifiers : Tableau des DIDs (identifiants de donn�es)
 * @param dataRecords : Tableau des enregistrements de donn�es associ�s aux DIDs
 * @param number_of_dids : Nombre de DIDs dans la r�ponse
 */
void send_positive_response_read_data_by_identifier(uint8_t* dataIdentifiers, uint8_t* dataRecords, uint8_t number_of_dids) {
    uint8_t response[MAX_DATA_SIZE] = {0};
    uint8_t index = 0;

    //response[index++] =
    response[index++] = UDS_READ_DATA_BY_IDENTIFIER + 0x40;  // SID de r�ponse 0x62

    // Ajouter les DIDs et leurs enregistrements associ�s
    for (uint8_t i = 0; i < number_of_dids; i++) {
        response[index++] = dataIdentifiers[2 * i];     // MSB du DID
        response[index++] = dataIdentifiers[2 * i + 1]; // LSB du DID
        response[index++] = dataRecords[2 * i];         // Valeur de l'enregistrement
        response[index++] = dataRecords[2 * i + 1];     // Valeur de l'enregistrement
    }

    // Envoyer la r�ponse via CAN
    send_can_message(response, index);
    // send_uart_message(response, index);
}

/**
 * @brief Envoie une r�ponse n�gative pour le service ReadDataByIdentifier (0x22)
 * @param nrc : Code de r�ponse n�gative (NRC)
 */
void send_negative_response_read_data_by_identifier(uint8_t nrc) {
    uint8_t response[8] = {0};

    response[0] = 4;
    response[1] = UDS_NEGATIVE_RESPONSE;  // SID pour r�ponse n�gative 0x7F
    response[2] = UDS_READ_DATA_BY_IDENTIFIER;  // SID du service 0x22
    response[3] = nrc;  // Code de r�ponse n�gative (NRC)

    send_can_message(response, 8);
    //send_uart_message(response, 3);
}

/*********************************************************ReadMemoryByAddress********************************************************/
// Simule une m�moire pour la lecture
uint8_t memory[1024] = {
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xAA, 0xBB,
    0xCC, 0xDD, 0xEE, 0xFF, 0xAA, 0xBB, 0xCC, 0xDD,
};

// Impl�mente le service ReadMemoryByAddress (0x23)
void uds_read_memory_by_address(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur minimale
    if (data_length < 4) {
        send_negative_response_read_memory_by_address(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t addressAndLengthFormatIdentifier = data[0];
    uint8_t address_length = addressAndLengthFormatIdentifier & 0x0F; // Bas nibble
    uint8_t size_length = (addressAndLengthFormatIdentifier >> 4) & 0x0F; // Haut nibble

    // V�rifier si l'ALFID est valide
    if (address_length < 1 || address_length > 4 || size_length < 1 || size_length > 4) {
        send_negative_response_read_memory_by_address(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Calculer l'adresse m�moire � partir des octets fournis
    uint32_t memory_address = 0;
    for (int i = 0; i < address_length; i++) {
        memory_address = (memory_address << 8) | data[1 + i];
    }

    // Calculer la taille m�moire � partir des octets fournis
    uint32_t memory_size = 0;
    for (int i = 0; i < size_length; i++) {
        memory_size = (memory_size << 8) | data[1 + address_length + i];
    }

    // V�rification de la plage d'adresses et de la taille
    if (memory_address + memory_size > sizeof(memory)) {
        send_negative_response_read_memory_by_address(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // V�rifier les conditions de s�curit�
    if (!uds_session.security_access_granted) {
        send_negative_response_read_memory_by_address(NRC_SECURITY_ACCESS_DENIED);
        return;
    }

    // Lire la m�moire et envoyer la r�ponse positive
    send_positive_response_read_memory_by_address(&memory[memory_address], memory_size);
}

// Fonction pour envoyer une r�ponse positive avec les donn�es lues
void send_positive_response_read_memory_by_address(uint8_t* dataRecord, uint8_t data_length) {
    uint8_t response[MAX_DATA_SIZE] = {0};
    uint8_t index = 0;

    // Ajouter le SID 0x63 pour la r�ponse positive
    response[index++] = UDS_READ_MEMORY_BY_ADDRESS + 0x40;

    // Ajouter les donn�es lues
    memcpy(&response[index], dataRecord, data_length);
    index += data_length;

    // Envoyer la r�ponse via CAN
    send_can_message(response, index);
    //send_uart_message(response, index);
}

// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_read_memory_by_address(uint8_t nrc) {
    uint8_t response[3] = {0};

    response[0] = UDS_NEGATIVE_RESPONSE;  // SID pour r�ponse n�gative 0x7F
    response[1] = UDS_READ_MEMORY_BY_ADDRESS;  // SID du service 0x23
    response[2] = nrc;  // Code de r�ponse n�gative (NRC)

    send_can_message(response, 3);
    //send_uart_message(response, 3);

}
/****************************************************ReadDataByPeriodicIdentifier***********************************************/
// Initialisation de la liste des PIDs p�riodiques
PeriodicPIDInfo periodic_pid_list[MAX_PERIODIC_PIDS];

// Fonction principale pour g�rer la requ�te ReadDataByPeriodicIdentifier
void uds_read_data_by_periodic_identifier(uint8_t* data, uint8_t data_length) {
    // V�rification de la longueur minimale du message
    if (data_length < 2) {
        send_negative_response_read_data_by_periodic_identifier(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t transmissionMode = data[1]; // Byte 2 : Mode de transmission
    if (transmissionMode < UDS_TRANSMISSION_MODE_SLOW || transmissionMode > UDS_TRANSMISSION_MODE_STOP) {
        send_negative_response_read_data_by_periodic_identifier(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // V�rification de la longueur pour chaque mode
    if (transmissionMode == UDS_TRANSMISSION_MODE_STOP && data_length < 2) {
        send_negative_response_read_data_by_periodic_identifier(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    } else if (transmissionMode != UDS_TRANSMISSION_MODE_STOP && data_length < 3) {
        send_negative_response_read_data_by_periodic_identifier(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Boucle pour traiter tous les periodicDataIdentifiers (PIDs) fournis
    for (uint8_t i = 2; i < data_length; i++) {
        uint8_t periodicDataIdentifier = data[i];

        // V�rifier si le PID est valide dans la session active
        if (!is_pid_supported_in_session(periodicDataIdentifier)) {
            send_negative_response_read_data_by_periodic_identifier(NRC_REQUEST_OUT_OF_RANGE);
            return;
        }

        // V�rifier la s�curit� si n�cessaire
        if (!is_security_granted_for_pid(periodicDataIdentifier)) {
            send_negative_response_read_data_by_periodic_identifier(NRC_SECURITY_ACCESS_DENIED);
            return;
        }

        // Traitement du mode stopSending
        if (transmissionMode == UDS_TRANSMISSION_MODE_STOP) {
            stop_periodic_transmission(periodicDataIdentifier);
        } else {
            // D�marrer la transmission p�riodique pour ce PID
            start_periodic_transmission(periodicDataIdentifier, transmissionMode);
        }
    }

    // Envoyer la r�ponse initiale positive
    send_positive_response_read_data_by_periodic_identifier();
}

// Fonction pour d�marrer la transmission p�riodique
void start_periodic_transmission(uint8_t pid, uint8_t mode) {
    // Parcourir la liste des PIDs p�riodiques pour trouver un emplacement libre
    for (int i = 0; i < MAX_PERIODIC_PIDS; i++) {
        if (!periodic_pid_list[i].isActive) {
            // Initialiser les param�tres pour ce PID
            periodic_pid_list[i].periodicDataIdentifier = pid;
            periodic_pid_list[i].transmissionMode = mode;
            periodic_pid_list[i].isActive = true;
            return;
        }
    }

    // Si aucun emplacement n'est libre, envoyer une r�ponse n�gative
    send_negative_response_read_data_by_periodic_identifier(NRC_CONDITIONS_NOT_CORRECT);
}

// Fonction pour arr�ter la transmission p�riodique pour un PID donn�
void stop_periodic_transmission(uint8_t pid) {
    for (int i = 0; i < MAX_PERIODIC_PIDS; i++) {
        if (periodic_pid_list[i].periodicDataIdentifier == pid && periodic_pid_list[i].isActive) {
            periodic_pid_list[i].isActive = false;
            return;
        }
    }
}

// Fonction pour arr�ter toutes les transmissions p�riodiques
void stop_all_periodic_transmissions(void) {
    for (int i = 0; i < MAX_PERIODIC_PIDS; i++) {
        periodic_pid_list[i].isActive = false;
    }
}

// Fonction pour envoyer une r�ponse positive initiale
void send_positive_response_read_data_by_periodic_identifier(void) {
    uint8_t response[2];
    response[0] = UDS_READ_DATA_BY_PERIODIC_IDENTIFIER + 0x40; // R�ponse positive SID
    response[1] = 0x00; // No data required in the positive response

    // Envoyer la r�ponse via CAN
    send_can_message(response, 2);
    //send_uart_message(response, 2);
}

// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_read_data_by_periodic_identifier(uint8_t nrc) {
    uint8_t response[3];
    response[0] = UDS_NEGATIVE_RESPONSE; // SID for negative response
    response[1] = UDS_READ_DATA_BY_PERIODIC_IDENTIFIER; // SID du service
    response[2] = nrc; // NRC code

    send_can_message(response, 3);
    //send_uart_message(response, 3);
}

// V�rification si un PID est support� dans la session active
bool is_pid_supported_in_session(uint8_t pid) {
    // V�rification des PIDs support�s (cette logique peut �tre modifi�e en fonction de la session)
    if (pid == 0xE3 || pid == 0x24 || pid == 0x01 || pid == 0x02) {
        return true;
    }
    return false;
}

// V�rification des autorisations de s�curit� pour un PID
bool is_security_granted_for_pid(uint8_t pid) {
    // Simuler une v�rification de s�curit�. Modifier cette fonction selon les besoins.
    // Par exemple, v�rifier si le pid n�cessite une session s�curis�e.
    return uds_session.security_access_granted;
}

/*************************************************DynamicallyDefineDataIdentifier***********************************************/
// Stockage des identifiants de donn�es dynamiques
static DynamicallyDefinedIdentifier dynamic_did_list[MAX_DYNAMIC_DIDS];
static uint8_t dynamic_did_count = 0;

// Fonction pour g�rer le service DynamicallyDefineDataIdentifier (0x2C)
void uds_dynamically_define_data_identifier(uint8_t sub_function, uint8_t *data, uint8_t data_length) {
    // V�rifier la sous-fonction demand�e
    switch (sub_function) {
        case UDS_DDDI_DEFINE_BY_IDENTIFIER:
            define_by_identifier(data, data_length);
            break;

        case UDS_DDDI_DEFINE_BY_MEMORY_ADDRESS:
            define_by_memory_address(data, data_length);
            break;

        case UDS_DDDI_CLEAR_DYNAMIC_IDENTIFIER:
            // Convertir les deux premiers octets en DID dynamique (uint16_t)
            if (data_length < 2) {
                send_negative_response_dynamically_define_data_identifier(sub_function, NRC_INCORRECT_MESSAGE_LENGTH);
                return;
            }
            uint16_t did_to_clear = (data[0] << 8) | data[1];
            clear_dynamically_defined_data_identifier(did_to_clear);
            break;

        default:
            // Envoyer une r�ponse n�gative pour une sous-fonction non support�e
            send_negative_response_dynamically_define_data_identifier(sub_function, NRC_SUB_FUNCTION_NOT_SUPPORTED);
            break;
    }
}

// Fonction pour d�finir un DID dynamique par identifiant (0x01)
void define_by_identifier(uint8_t *data, uint8_t data_length) {
    if (data_length < 8) {
        // Longueur de message incorrecte
        send_negative_response_dynamically_define_data_identifier(UDS_DDDI_DEFINE_BY_IDENTIFIER, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    if (dynamic_did_count >= MAX_DYNAMIC_DIDS) {
        // D�passement du nombre maximum d'identifiants dynamiques
        send_negative_response_dynamically_define_data_identifier(UDS_DDDI_DEFINE_BY_IDENTIFIER, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Extraire les donn�es de la requ�te
    uint16_t dynamic_did = (data[0] << 8) | data[1];   // DID dynamique
    uint16_t source_did = (data[2] << 8) | data[3];    // DID source
    uint8_t position_in_source = data[4];              // Position dans le DID source
    uint8_t memory_size = data[5];                     // Taille des donn�es

    // V�rification si le DID est d�j� d�fini
    if (is_dynamic_identifier_supported(dynamic_did)) {
        send_negative_response_dynamically_define_data_identifier(UDS_DDDI_DEFINE_BY_IDENTIFIER, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Enregistrer le nouveau DID dynamique
    dynamic_did_list[dynamic_did_count].dynamicDataIdentifier = dynamic_did;
    dynamic_did_list[dynamic_did_count].sourceDataIdentifier = source_did;
    dynamic_did_list[dynamic_did_count].positionInSource = position_in_source;
    dynamic_did_list[dynamic_did_count].memorySize = memory_size;

    dynamic_did_count++;

    // Envoyer une r�ponse positive
    send_positive_response_dynamically_define_data_identifier(UDS_DDDI_DEFINE_BY_IDENTIFIER, dynamic_did);
}

// Fonction pour d�finir un DID dynamique par adresse m�moire (0x02)
void define_by_memory_address(uint8_t *data, uint8_t data_length) {
    if (data_length < 10) {
        send_negative_response_dynamically_define_data_identifier(UDS_DDDI_DEFINE_BY_MEMORY_ADDRESS, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    if (dynamic_did_count >= MAX_DYNAMIC_DIDS) {
        send_negative_response_dynamically_define_data_identifier(UDS_DDDI_DEFINE_BY_MEMORY_ADDRESS, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Extraire les informations de la requ�te
    uint16_t dynamic_did = (data[0] << 8) | data[1];   // DID dynamique
    uint8_t address_format = data[2];                  // Format d'adresse et de taille

    // Validation simple bas�e sur address_format (Exemple)
    if (address_format != 0x14) {
        send_negative_response_dynamically_define_data_identifier(UDS_DDDI_DEFINE_BY_MEMORY_ADDRESS, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    uint32_t memory_address = (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6]; // Adresse m�moire
    uint8_t memory_size = data[7];                     // Taille des donn�es

    // Enregistrer le DID dynamique
    dynamic_did_list[dynamic_did_count].dynamicDataIdentifier = dynamic_did;
    dynamic_did_list[dynamic_did_count].positionInSource = memory_address; // Stocker l'adresse comme position
    dynamic_did_list[dynamic_did_count].memorySize = memory_size;

    dynamic_did_count++;

    // Envoyer une r�ponse positive
    send_positive_response_dynamically_define_data_identifier(UDS_DDDI_DEFINE_BY_MEMORY_ADDRESS, dynamic_did);
}


// Fonction pour effacer un DID dynamique (0x03)
void clear_dynamically_defined_data_identifier(uint16_t dynamicDataIdentifier) {
    // Rechercher et supprimer le DID dynamique
    for (uint8_t i = 0; i < dynamic_did_count; i++) {
        if (dynamic_did_list[i].dynamicDataIdentifier == dynamicDataIdentifier) {
            // Effacer l'entr�e en d�calant les autres �l�ments
            dynamic_did_list[i] = dynamic_did_list[dynamic_did_count - 1]; // Remplacer par la derni�re entr�e
            dynamic_did_count--;
            send_positive_response_dynamically_define_data_identifier(UDS_DDDI_CLEAR_DYNAMIC_IDENTIFIER, dynamicDataIdentifier);
            return;
        }
    }

    // Si le DID n'existe pas, envoyer une r�ponse n�gative
    send_negative_response_dynamically_define_data_identifier(UDS_DDDI_CLEAR_DYNAMIC_IDENTIFIER, NRC_REQUEST_OUT_OF_RANGE);
}

// Fonction pour v�rifier si un DID dynamique est d�j� d�fini
bool is_dynamic_identifier_supported(uint16_t did) {
    for (uint8_t i = 0; i < dynamic_did_count; i++) {
        if (dynamic_did_list[i].dynamicDataIdentifier == did) {
            return true;
        }
    }
    return false;
}

// Fonction pour envoyer une r�ponse positive
void send_positive_response_dynamically_define_data_identifier(uint8_t sub_function, uint16_t dynamic_did) {
    uint8_t response[4];
    response[0] = UDS_DYNAMICAL_DEFINE_DATA_IDENTIFIER + 0x40; // SID pour la r�ponse positive
    response[1] = sub_function;
    response[2] = (dynamic_did >> 8) & 0xFF; // DID MSB
    response[3] = dynamic_did & 0xFF;        // DID LSB
    send_can_message(response, 4);
    //send_uart_message(response, 4);
}

// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_dynamically_define_data_identifier(uint8_t sub_function, uint8_t nrc) {
    uint8_t response[3];
    response[0] = UDS_NEGATIVE_RESPONSE; // R�ponse n�gative
    response[1] = UDS_DYNAMICAL_DEFINE_DATA_IDENTIFIER; // SID du service
    response[2] = nrc; // Code NRC
    send_can_message(response, 3);
    //send_uart_message(response, 3);
}
/************************************************* WriteMemoryByAddress*********************************************************/

// Stockage fictif pour les DIDs, � remplacer par la logique r�elle de votre syst�me
uint8_t did_storage[MAX_DATA_SIZE] = {0};

// Fonction principale pour g�rer le service WriteDataByIdentifier (0x2E)
void uds_write_data_by_identifier(uint8_t* data, uint8_t data_length) {
    // V�rification de la longueur minimale du message (4 octets : SID + DID + au moins 1 octet de donn�es)
    if (data_length < 4) {
        send_negative_response_write_data_by_identifier(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Extraire le dataIdentifier (DID) de la requ�te
    uint16_t dataIdentifier = (data[0] << 8) | data[1];

    // V�rifier si le DID est support� et writable dans la session actuelle
    if (!is_data_identifier_supported_for_write(dataIdentifier)) {
        send_negative_response_write_data_by_identifier(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // V�rification de la s�curit� (si le DID n�cessite un acc�s s�curis�)
    if (is_security_required_for_did(dataIdentifier) && !uds_session.security_access_granted) {
        send_negative_response_write_data_by_identifier(NRC_SECURITY_ACCESS_DENIED);
        return;
    }

    // V�rifier les conditions d'�criture (par ex. session, �tat de l'ECU)
    if (!are_conditions_correct_for_did(dataIdentifier)) {
        send_negative_response_write_data_by_identifier(NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Extraire le dataRecord � partir du message
    uint8_t* dataRecord = &data[2];
    uint8_t dataRecordLength = data_length - 2;

    // Effectuer l'�criture des donn�es dans le DID sp�cifi�
    if (!write_data_to_identifier(dataIdentifier, dataRecord, dataRecordLength)) {
        send_negative_response_write_data_by_identifier(NRC_GENERAL_PROGRAMMING_FAILURE);
        return;
    }

    // Envoyer une r�ponse positive apr�s �criture r�ussie
    send_positive_response_write_data_by_identifier(dataIdentifier);
}

// Fonction pour v�rifier si un DID est support� pour l'�criture
bool is_data_identifier_supported_for_write(uint16_t dataIdentifier) {
    // Impl�mentez la logique ici pour v�rifier si le DID est writable
    switch (dataIdentifier) {
        case SUPPORTED_DID_1:
        case SUPPORTED_DID_2:
        case SUPPORTED_DID_3:
            return true;  // Ces DIDs supportent l'�criture
        default:
            return false; // DID non support�
    }
}



// Fonction pour �crire des donn�es dans le DID sp�cifi�
bool write_data_to_identifier(uint16_t dataIdentifier, uint8_t* dataRecord, uint8_t dataLength) {
    // Impl�mentez la logique pour �crire les donn�es dans la m�moire associ�e au DID
    // Exemple simple : �crire dans une zone de m�moire fictive
    switch (dataIdentifier) {
        case SUPPORTED_DID_1:
            memcpy(&did_storage[0], dataRecord, dataLength);
            return true;
        case SUPPORTED_DID_2:
            memcpy(&did_storage[16], dataRecord, dataLength);
            return true;
        case SUPPORTED_DID_3:
            memcpy(&did_storage[32], dataRecord, dataLength);
            return true;
        default:
            return false;
    }
}

// Fonction pour envoyer une r�ponse positive apr�s �criture r�ussie
void send_positive_response_write_data_by_identifier(uint16_t dataIdentifier) {
    uint8_t response[3];
    response[0] = UDS_WRITE_DATA_BY_IDENTIFIER + 0x40; // R�ponse positive SID (0x2E + 0x40 = 0x6E)
    response[1] = (dataIdentifier >> 8) & 0xFF;       // Octet MSB du dataIdentifier
    response[2] = dataIdentifier & 0xFF;              // Octet LSB du dataIdentifier
    send_can_message(response, 3);
    //send_uart_message(response, 3);
}

// Fonction pour envoyer une r�ponse n�gative pour WriteDataByIdentifier
void send_negative_response_write_data_by_identifier(uint8_t nrc) {
    uint8_t response[3];
    response[0] = UDS_NEGATIVE_RESPONSE;   // SID pour une r�ponse n�gative
    response[1] = UDS_WRITE_DATA_BY_IDENTIFIER; // SID du service WriteDataByIdentifier (0x2E)
    response[2] = nrc;                     // Code NRC
    send_can_message(response, 3);
    //send_uart_message(response, 3);
}

// Fonction pour v�rifier les conditions sp�cifiques (exemple : �tat de la session)
bool are_conditions_correct_for_did(uint16_t dataIdentifier) {
    // Impl�mentez des v�rifications suppl�mentaires si n�cessaire
    // Exemple : autorisation seulement dans une session �tendue
    if (uds_session.current_session == 0) {  // Session par d�faut
        return false;  // Conditions non remplies si la session est par d�faut
    }
    return true;
}

void send_can_message(uint8_t *message, uint8_t length) {
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;

    TxHeader.DLC = length;
    TxHeader.StdId = 0x7E8; // Identifiant standard UDS pour l'ECU
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;

    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, message, &TxMailbox) != HAL_OK) {
        // G�rer l'erreur d'envoi
        Error_handler();
    }
}

/*
 * void send_uart_message(uint8_t *message, uint8_t length) {
    if (HAL_UART_Transmit(&huart1, message, length, HAL_MAX_DELAY) != HAL_OK) {
        // G�rer l'erreur d'envoi
        Error_handler();
    }
}
 *
 */



