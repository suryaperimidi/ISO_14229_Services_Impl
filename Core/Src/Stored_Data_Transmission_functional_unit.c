/*
 * c.c
 *
 *  Created on: Jul 9, 2025
 *      Author: suneel.kataraki
 */


#include "Stored_Data_Transmission_functional_unit.h"
#include "uds_services.c"
#include"main.h"

/************************************************ClearDiagnosticInformation*************************************************/
// Fonction principale pour ClearDiagnosticInformation
void uds_clear_diagnostic_information(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur du message (SID + groupOfDTC = 4 octets)
    if (data_length != 4) {
        send_negative_response_clear_diagnostic_information(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Extraire le groupOfDTC (3 octets)
    uint32_t groupOfDTC = (data[1] << 16) | (data[2] << 8) | data[3];

    // V�rifier si le groupOfDTC est support�
    if (!is_group_of_dtc_supported(groupOfDTC)) {
        send_negative_response_clear_diagnostic_information(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // V�rifier si les conditions pour effacer les DTC sont correctes
    if (!are_conditions_correct_for_dtc_clear()) {
        send_negative_response_clear_diagnostic_information(NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Effacer les informations diagnostiques pour le groupOfDTC
    if (!clear_diagnostic_information(groupOfDTC)) {
        send_negative_response_clear_diagnostic_information(NRC_GENERAL_PROGRAMMING_FAILURE);
        return;
    }

    // Si tout est correct, envoyer une r�ponse positive
    send_positive_response_clear_diagnostic_information();
}

// V�rifier si le groupOfDTC est support�
bool is_group_of_dtc_supported(uint32_t groupOfDTC) {
    // Impl�menter la logique pour v�rifier si le groupOfDTC est support�
    // Par exemple, supposons que nous supportons uniquement certains groupes de DTC
    switch (groupOfDTC) {
        case 0x000000:  // Exemples de groupes de DTC
        case 0x010000:  // Groupe Powertrain
        case 0x020000:  // Groupe Chassis
        case 0x030000:  // Groupe Body
            return true;
        default:
            return false;
    }
}

// V�rifier si les conditions pour effacer les DTC sont correctes
bool are_conditions_correct_for_dtc_clear(void) {
    // Impl�menter les v�rifications sp�cifiques, par exemple :
    // - Le v�hicule est-il dans un mode qui permet l'effacement des DTC ?
    // - La session diagnostique actuelle est-elle correcte ?

    if (uds_session.current_session == UDS_SESSION_DEFAULT) {
        return false;  // L'effacement n'est pas autoris� dans la session par d�faut
    }
    // Ajoutez d'autres v�rifications selon les besoins (�tat moteur, etc.)

    return true;  // Toutes les conditions sont correctes
}

// Effacer les informations diagnostiques pour le groupOfDTC sp�cifi�
bool clear_diagnostic_information(uint32_t groupOfDTC) {
    // Impl�menter la logique pour effacer les informations DTC
    // Ici, nous simulons simplement l'effacement, mais dans un vrai syst�me,
    // cela inclurait des op�rations sur la m�moire et des bases de donn�es de DTC.

    // Exemple : Supposons que l'effacement r�ussit toujours pour cette d�monstration
    return true;

    // En cas d'�chec (par exemple, �chec d'�criture en m�moire), renvoyer false
}

// Envoyer une r�ponse positive apr�s effacement r�ussi
void send_positive_response_clear_diagnostic_information(void) {
    uint8_t response[1];
    response[0] = UDS_CLEAR_DIAGNOSTIC_INFORMATION + 0x40;  // SID de r�ponse positive (0x14 + 0x40 = 0x54)
    send_can_message(response, 1);  // Envoyer la r�ponse sur le bus CAN
    // send_uart_message(response, 1);
}

// Envoyer une r�ponse n�gative en cas d'erreur
void send_negative_response_clear_diagnostic_information(uint8_t nrc) {
    uint8_t response[3];
    response[0] = UDS_NEGATIVE_RESPONSE;  // SID pour une r�ponse n�gative
    response[1] = UDS_CLEAR_DIAGNOSTIC_INFORMATION;  // SID de ClearDiagnosticInformation (0x14)
    response[2] = nrc;  // Code de r�ponse n�gative (NRC)
    send_can_message(response, 3);  // Envoyer la r�ponse sur le bus CAN
    // send_uart_message(response, 3);
}


/************************************************read_dtc_information*******************************************************/
DTC_Record stored_dtc_list[MAX_DTC_COUNT];  // Liste des DTC stock�s
DTC_Record mirror_dtc_list[MAX_DTC_COUNT];
DTC_Record user_defined_memory_list[MAX_DTC_COUNT];


// Fonction principale du service ReadDTCInformation
void uds_read_dtc_information(uint8_t sub_function, uint8_t* data, uint8_t data_length) {
    switch (sub_function) {
        case REPORT_NUMBER_OF_DTC_BY_STATUS_MASK:
            report_number_of_dtc_by_status_mask(data, data_length);
            break;
        case REPORT_DTC_BY_STATUS_MASK:
            report_dtc_by_status_mask(data, data_length);
            break;
        case REPORT_DTC_SNAPSHOT_IDENTIFICATION:
            report_dtc_snapshot_identification(data, data_length);
            break;
        case REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER:
            report_dtc_snapshot_record_by_dtc_number(data, data_length);
            break;
        case REPORT_DTC_STORED_DATA_BY_RECORD_NUMBER:
            report_dtc_stored_data_by_record_number(data, data_length);
            break;
        case REPORT_DTC_EXT_DATA_RECORD_BY_DTC_NUMBER:
            report_dtc_ext_data_record_by_dtc_number(data, data_length);
            break;
        case REPORT_NUMBER_OF_DTC_BY_SEVERITY_MASK:
            report_number_of_dtc_by_severity_mask(data, data_length);
            break;
        case REPORT_DTC_BY_SEVERITY_MASK_RECORD:
            report_dtc_by_severity_mask_record(data, data_length);
            break;
        case REPORT_SEVERITY_INFORMATION_OF_DTC:
            report_severity_information_of_dtc(data, data_length);
            break;
        case REPORT_SUPPORTED_DTC:
            report_supported_dtc(data, data_length);
            break;
        case REPORT_FIRST_TEST_FAILED_DTC:
            report_first_test_failed_dtc(data, data_length);
            break;
        case REPORT_FIRST_CONFIRMED_DTC:
            report_first_confirmed_dtc(data, data_length);
            break;
        case REPORT_MOST_RECENT_TEST_FAILED_DTC:
            report_most_recent_test_failed_dtc(data, data_length);
            break;
        case REPORT_MOST_RECENT_CONFIRMED_DTC:
            report_most_recent_confirmed_dtc(data, data_length);
            break;
        case REPORT_MIRROR_MEMORY_DTC_BY_STATUS_MASK:
            report_mirror_memory_dtc_by_status_mask(data, data_length);
            break;
        case REPORT_MIRROR_MEMORY_DTC_EXT_DATA_RECORD:
            report_mirror_memory_dtc_ext_data_record(data, data_length);
            break;
        case REPORT_NUMBER_OF_MIRROR_MEMORY_DTC_BY_STATUS_MASK:
            report_number_of_mirror_memory_dtc_by_status_mask(data, data_length);
            break;
        case REPORT_NUMBER_OF_EMISSIONS_OBD_DTC_BY_STATUS_MASK:
            report_number_of_emissions_obd_dtc_by_status_mask(data, data_length);
            break;
        case REPORT_EMISSIONS_OBD_DTC_BY_STATUS_MASK:
            report_emissions_obd_dtc_by_status_mask(data, data_length);
            break;
        case REPORT_DTC_FAULT_DETECTION_COUNTER:
            report_dtc_fault_detection_counter(data, data_length);
            break;
        case REPORT_DTC_WITH_PERMANENT_STATUS:
            report_dtc_with_permanent_status(data, data_length);
            break;
        case REPORT_DTC_EXT_DATA_RECORD_BY_RECORD_NUMBER:
            report_dtc_ext_data_record_by_record_number(data, data_length);
            break;
        case REPORT_USER_DEF_MEMORY_DTC_BY_STATUS_MASK:
            report_user_def_memory_dtc_by_status_mask(data, data_length);
            break;
        case REPORT_USER_DEF_MEMORY_DTC_SNAPSHOT_RECORD:
            report_user_def_memory_dtc_snapshot_record(data, data_length);
            break;
        case REPORT_USER_DEF_MEMORY_DTC_EXT_DATA_RECORD:
            report_user_def_memory_dtc_ext_data_record(data, data_length);
            break;
        case REPORT_WWH_OBD_DTC_BY_MASK_RECORD:
            report_wwh_obd_dtc_by_mask_record(data, data_length);
            break;
        case REPORT_WWH_OBD_DTC_WITH_PERMANENT_STATUS:
            report_wwh_obd_dtc_with_permanent_status(data, data_length);
            break;
        default:
            send_negative_response_read_dtc_information(sub_function, NRC_SUB_FUNCTION_NOT_SUPPORTED);
            break;
    }
}

void send_positive_response_read_dtc_information(uint8_t sub_function, DTC_Record* dtcRecords, uint8_t dtcCount) {
    // Limiter la taille maximale de la r�ponse � la taille d'un message CAN (par exemple, 8 octets)
    uint8_t response[8];  // Tableau de r�ponse limit� � 8 octets
    uint8_t index = 0;

    // Champ 1 : SID pour ReadDTCInformation
    response[index++] = 0x59;  // SID pour ReadDTCInformation (r�ponse positive)

    // Champ 2 : Type de rapport (sub_function)
    response[index++] = sub_function;

    switch (sub_function) {
        case REPORT_NUMBER_OF_DTC_BY_STATUS_MASK:
        case REPORT_NUMBER_OF_DTC_BY_SEVERITY_MASK:
            // Champs 3 : DTCStatusAvailabilityMask
            response[index++] = get_dtc_status_availability_mask();

            // Champs 4 : Nombre de DTCs
            response[index++] = (dtcCount >> 8) & 0xFF;  // Octet �lev� du nombre de DTCs
            response[index++] = dtcCount & 0xFF;         // Octet bas du nombre de DTCs
            break;

        case REPORT_DTC_BY_STATUS_MASK:
        case REPORT_SUPPORTED_DTC:
        case REPORT_FIRST_TEST_FAILED_DTC:
            // Ajouter les informations sur chaque DTC, envoyer en plusieurs messages si n�cessaire
            for (uint8_t i = 0; i < dtcCount; i++) {
                index = 2;  // R�initialiser l'index apr�s SID et sous-fonction pour chaque nouveau message

                // Champs DTC
                response[index++] = (dtcRecords[i].dtcNumber >> 16) & 0xFF;  // Octet sup�rieur du DTC
                response[index++] = (dtcRecords[i].dtcNumber >> 8) & 0xFF;   // Octet du milieu
                response[index++] = dtcRecords[i].dtcNumber & 0xFF;          // Octet inf�rieur
                response[index++] = dtcRecords[i].status;                    // Statut du DTC

                // Si la r�ponse est compl�te (7 octets max pour un message CAN), envoyer le message
                send_can_message(response, index);

                // R�initialiser le tableau `response` pour le prochain DTC
                index = 0;
            }
            return;  // Tous les messages ont �t� envoy�s
            break;

        default:
            send_negative_response_read_dtc_information(sub_function, NRC_SUB_FUNCTION_NOT_SUPPORTED);
            return;
    }

    // Envoi du message si tous les octets tiennent dans un seul message CAN
    send_can_message(response, index);
    // send_uart_message(response, 3);
}




// Envoi d'une r�ponse n�gative pour ReadDTCInformation
void send_negative_response_read_dtc_information(uint8_t sub_function, uint8_t nrc) {
    uint8_t response[3];
    response[0] = 0x7F; // Negative Response SID
    response[1] = sub_function;
    response[2] = nrc;
    send_can_message(response, 3);
    // send_uart_message(response, 3);
}

// Impl�mentation des sous-fonctions ReadDTCInformation
void report_number_of_dtc_by_status_mask(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur des donn�es re�ues
    if (data_length != 1) {
        send_negative_response_read_dtc_information(REPORT_NUMBER_OF_DTC_BY_STATUS_MASK, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t status_mask = data[0];  // Le masque de statut est envoy� en premier
    uint16_t dtc_count = 0;         // Compteur de DTC
    uint8_t dtc_status_availability_mask = get_dtc_status_availability_mask();
    uint8_t dtc_format_identifier = get_dtc_format_identifier();

    // Compter les DTC qui correspondent au masque de statut
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if ((stored_dtc_list[i].status & status_mask) != 0) {
            dtc_count++;
        }
    }

    // Si aucun DTC n'a �t� trouv�, renvoyer un NRC (aucune condition remplie)
    if (dtc_count == 0) {
        send_negative_response_read_dtc_information(REPORT_NUMBER_OF_DTC_BY_STATUS_MASK, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Allouer dynamiquement la m�moire pour la r�ponse (4 octets)
    uint8_t* response = (uint8_t*)malloc(4 * sizeof(uint8_t));
    if (response == NULL) {
        // Si l'allocation �choue, g�rer l'erreur
        Error_handler();
        return;
    }

    // Cr�er la r�ponse avec le masque de disponibilit� et l'identifiant du format de DTC
    response[0] = dtc_status_availability_mask;
    response[1] = dtc_format_identifier;
    response[2] = (dtc_count >> 8) & 0xFF;  // Octet de poids fort du compteur de DTC
    response[3] = dtc_count & 0xFF;         // Octet de poids faible du compteur de DTC

    // Envoyer la r�ponse positive au client via le CAN (ou autre protocole de communication)
    send_can_message(response, 4);  // Utiliser send_can_message pour envoyer la r�ponse sous forme de tableau d'octets
    // send_uart_message(response, 4);
    // Lib�rer la m�moire allou�e dynamiquement
    free(response);
}

void report_dtc_by_status_mask(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur des donn�es re�ues
    if (data_length != 1) {
        send_negative_response_read_dtc_information(REPORT_DTC_BY_STATUS_MASK, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t status_mask = data[0];  // Le masque de statut est envoy� en premier
    uint16_t dtc_count = 0;         // Compteur de DTC

    // Compter le nombre de DTC correspondant au masque de statut
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if ((stored_dtc_list[i].status & status_mask) != 0) {
            dtc_count++;
        }
    }

    // Si aucun DTC n'a �t� trouv�, renvoyer un NRC (aucune condition remplie)
    if (dtc_count == 0) {
        send_negative_response_read_dtc_information(REPORT_DTC_BY_STATUS_MASK, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Allouer dynamiquement de la m�moire pour les DTC_Record
    DTC_Record* dtc_records = (DTC_Record*)malloc(dtc_count * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        // Si l'allocation �choue, g�rer l'erreur
        Error_handler();
        return;
    }

    // Remplir les enregistrements DTC
    uint16_t index = 0;
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if ((stored_dtc_list[i].status & status_mask) != 0 && index < dtc_count) {
            dtc_records[index].dtcNumber = stored_dtc_list[i].dtcNumber;
            dtc_records[index].status = stored_dtc_list[i].status;
            dtc_records[index].severity = stored_dtc_list[i].severity;
            index++;
        }
    }

    // Envoyer la r�ponse positive avec les DTC_Record
    send_positive_response_read_dtc_information(REPORT_DTC_BY_STATUS_MASK, dtc_records, dtc_count);

    // Lib�rer la m�moire allou�e dynamiquement
    free(dtc_records);
}



void report_dtc_snapshot_identification(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur des donn�es re�ues
    if (data_length != 0) {  // Aucune donn�e n'est attendue pour ce service
        send_negative_response_read_dtc_information(REPORT_DTC_SNAPSHOT_IDENTIFICATION, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint16_t dtc_count = 0;  // Compteur pour le nombre de DTC
    uint16_t snapshot_count = 0;  // Compteur pour le nombre de snapshots

    // Compter le nombre de DTCs qui ont un snapshot associ�
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].snapshotDataLength > 0) {  // Un snapshot est pr�sent pour ce DTC
            snapshot_count += stored_dtc_list[i].snapshotRecordNumber;  // Compter chaque occurrence
            dtc_count++;
        }
    }

    // Si aucun snapshot n'est trouv�, renvoyer un NRC
    if (snapshot_count == 0) {
        send_negative_response_read_dtc_information(REPORT_DTC_SNAPSHOT_IDENTIFICATION, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Allouer dynamiquement de la m�moire pour les enregistrements DTC_Record
    DTC_Record* dtc_records = (DTC_Record*)malloc(snapshot_count * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        // Si l'allocation �choue, g�rer l'erreur
        Error_handler();
        return;
    }

    // Remplir les enregistrements DTC_Record avec le num�ro DTC et le num�ro de snapshot
    uint16_t index = 0;
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].snapshotDataLength > 0) {
            // Pour chaque occurrence de snapshot pour un DTC donn�
            for (uint8_t j = 0; j < stored_dtc_list[i].snapshotRecordNumber; j++) {
                dtc_records[index].dtcNumber = stored_dtc_list[i].dtcNumber;
                dtc_records[index].snapshotRecordNumber = j + 1;  // Num�rotation des snapshots
                index++;
            }
        }
    }

    // Envoyer la r�ponse positive avec les enregistrements DTC_Record
    send_positive_response_read_dtc_information(REPORT_DTC_SNAPSHOT_IDENTIFICATION, dtc_records, snapshot_count);

    // Lib�rer la m�moire allou�e dynamiquement
    free(dtc_records);
}



void report_dtc_snapshot_record_by_dtc_number(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur des donn�es re�ues (DTCMaskRecord + SnapshotRecordNumber = 4 octets attendus)
    if (data_length != 4) {
        send_negative_response_read_dtc_information(REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Extraire le num�ro de DTC depuis la requ�te du client (3 octets)
    uint32_t dtc_mask_record = (data[0] << 16) | (data[1] << 8) | data[2];
    uint8_t snapshot_record_number = data[3];  // Num�ro d'enregistrement du snapshot

    uint8_t found = 0;
    DTC_Record dtc_record;  // Pas d'allocation dynamique pour DTC_Record

    // Parcourir la liste des DTCs pour trouver le DTC correspondant au masque demand�
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].dtcNumber == dtc_mask_record) {
            // Si un enregistrement de snapshot est trouv�
            if ((stored_dtc_list[i].snapshotRecordNumber >= snapshot_record_number && snapshot_record_number != 0xFF) || snapshot_record_number == 0xFF) {
                dtc_record.dtcNumber = stored_dtc_list[i].dtcNumber;
                dtc_record.status = stored_dtc_list[i].status;
                dtc_record.snapshotRecordNumber = snapshot_record_number;

                // Remplir les donn�es de snapshot
                for (uint8_t j = 0; j < stored_dtc_list[i].snapshotDataLength; j++) {
                    dtc_record.snapshotData[j] = stored_dtc_list[i].snapshotData[j];  // Utiliser le tableau fixe
                }
                dtc_record.snapshotDataLength = stored_dtc_list[i].snapshotDataLength;
                found = 1;
                break;
            }
        }
    }

    // Si aucun enregistrement n'est trouv�, renvoyer un NRC
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Envoyer une r�ponse positive avec l'enregistrement DTC et le snapshot correspondant
    send_positive_response_read_dtc_information(REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER, &dtc_record, 1);
}



void report_dtc_stored_data_by_record_number(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur des donn�es re�ues
    if (data_length != 2) {
        send_negative_response_read_dtc_information(REPORT_DTC_STORED_DATA_BY_RECORD_NUMBER, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint16_t dtc_stored_data_record_number = (data[0] << 8) | data[1];

    uint8_t found = 0;
    DTC_Record dtc_record;  // Pas d'allocation dynamique ici

    // Parcourir la liste des DTCs pour trouver l'enregistrement correspondant au num�ro fourni
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].storedDataRecordNumber == dtc_stored_data_record_number || dtc_stored_data_record_number == 0xFF) {
            dtc_record.dtcNumber = stored_dtc_list[i].dtcNumber;
            dtc_record.status = stored_dtc_list[i].status;
            dtc_record.storedDataRecordNumber = stored_dtc_list[i].storedDataRecordNumber;

            // Copie les donn�es dans le tableau de taille fixe
            for (uint8_t j = 0; j < stored_dtc_list[i].storedDataLength; j++) {
                dtc_record.snapshotData[j] = stored_dtc_list[i].storedData[j];
            }
            dtc_record.snapshotDataLength = stored_dtc_list[i].storedDataLength;
            found = 1;
            break;
        }
    }

    // Si aucun enregistrement n'est trouv�
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_DTC_STORED_DATA_BY_RECORD_NUMBER, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Envoyer une r�ponse positive avec les donn�es trouv�es
    send_positive_response_read_dtc_information(REPORT_DTC_STORED_DATA_BY_RECORD_NUMBER, &dtc_record, 1);
}



void report_dtc_ext_data_record_by_dtc_number(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur des donn�es re�ues (DTCMaskRecord + ExtDataRecordNumber = 4 octets attendus)
    if (data_length != 4) {
        send_negative_response_read_dtc_information(REPORT_DTC_EXT_DATA_RECORD_BY_DTC_NUMBER, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Extraire le num�ro de DTC depuis la requ�te du client (3 octets)
    uint32_t dtc_mask_record = (data[0] << 16) | (data[1] << 8) | data[2];
    uint8_t ext_data_record_number = data[3];  // Num�ro d'enregistrement des donn�es �tendues

    uint8_t found = 0;
    DTC_Record dtc_record;

    // Parcourir la liste des DTCs pour trouver le DTC correspondant au masque et au num�ro de donn�es �tendues
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].dtcNumber == dtc_mask_record) {
            // Si un enregistrement de donn�es �tendues est trouv�
            if (stored_dtc_list[i].storedDataRecordNumber == ext_data_record_number || ext_data_record_number == 0xFF) {
                dtc_record.dtcNumber = stored_dtc_list[i].dtcNumber;
                dtc_record.status = stored_dtc_list[i].status;
                dtc_record.storedDataRecordNumber = stored_dtc_list[i].storedDataRecordNumber;

                // Copier les donn�es �tendues directement dans le tableau `snapshotData`
                dtc_record.snapshotDataLength = stored_dtc_list[i].storedDataLength;
                if (dtc_record.snapshotDataLength > MAX_DTC_EXT_DATA_RECORD_SIZE) {
                    send_negative_response_read_dtc_information(REPORT_DTC_EXT_DATA_RECORD_BY_DTC_NUMBER, NRC_REQUEST_OUT_OF_RANGE);
                    return;
                }
                for (uint8_t j = 0; j < dtc_record.snapshotDataLength; j++) {
                    dtc_record.snapshotData[j] = stored_dtc_list[i].storedData[j];
                }

                found = 1;
                break;
            }
        }
    }

    // Si aucun enregistrement n'est trouv�, renvoyer un NRC
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_DTC_EXT_DATA_RECORD_BY_DTC_NUMBER, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Envoyer une r�ponse positive avec l'enregistrement DTC et les donn�es �tendues correspondantes
    send_positive_response_read_dtc_information(REPORT_DTC_EXT_DATA_RECORD_BY_DTC_NUMBER, &dtc_record, 1);
}



void report_number_of_dtc_by_severity_mask(uint8_t* data, uint8_t data_length) {
    // V�rifier que la longueur des donn�es est correcte (statut + gravit� = 2 octets attendus)
    if (data_length != 2) {
        send_negative_response_read_dtc_information(REPORT_NUMBER_OF_DTC_BY_SEVERITY_MASK, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t dtc_status_mask = data[0];
    uint8_t dtc_severity_mask = data[1];
    uint16_t dtc_count = 0;

    // Parcourir tous les DTCs et compter ceux qui correspondent au masque de statut et de gravit�
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (((stored_dtc_list[i].status & dtc_status_mask) != 0) &&
            ((stored_dtc_list[i].severity & dtc_severity_mask) != 0)) {
            dtc_count++;
        }
    }

    // Cr�er la r�ponse
    uint8_t response[3];
    response[0] = get_dtc_status_availability_mask();
    response[1] = (dtc_count >> 8) & 0xFF;  // Octet de poids fort
    response[2] = dtc_count & 0xFF;         // Octet de poids faible

    // Envoyer la r�ponse au client
    send_positive_response_read_dtc_information(REPORT_NUMBER_OF_DTC_BY_SEVERITY_MASK, (DTC_Record*)response, 0);
}




void report_dtc_by_severity_mask_record(uint8_t* data, uint8_t data_length) {
    // V�rifier que la longueur des donn�es est correcte (statut + gravit� = 2 octets attendus)
    if (data_length != 2) {
        send_negative_response_read_dtc_information(REPORT_DTC_BY_SEVERITY_MASK_RECORD, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t dtc_status_mask = data[0];
    uint8_t dtc_severity_mask = data[1];
    uint8_t found_dtc_count = 0;

    // Allouer dynamiquement de la m�moire pour les enregistrements DTC
    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    // Parcourir tous les DTCs et ajouter ceux qui correspondent au masque de statut et de gravit�
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (((stored_dtc_list[i].status & dtc_status_mask) != 0) &&
            ((stored_dtc_list[i].severity & dtc_severity_mask) != 0)) {
            dtc_records[found_dtc_count] = stored_dtc_list[i];
            found_dtc_count++;
        }
    }

    // Si aucun DTC n'a �t� trouv�, envoyer un NRC
    if (found_dtc_count == 0) {
        free(dtc_records);
        send_negative_response_read_dtc_information(REPORT_DTC_BY_SEVERITY_MASK_RECORD, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Envoyer la r�ponse avec les enregistrements DTC correspondants
    send_positive_response_read_dtc_information(REPORT_DTC_BY_SEVERITY_MASK_RECORD, dtc_records, found_dtc_count);

    // Lib�rer la m�moire allou�e
    free(dtc_records);
}



void report_severity_information_of_dtc(uint8_t* data, uint8_t data_length) {
    // V�rifier que la longueur des donn�es est correcte (3 octets pour le num�ro de DTC)
    if (data_length != 3) {
        send_negative_response_read_dtc_information(REPORT_SEVERITY_INFORMATION_OF_DTC, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Extraire le num�ro de DTC depuis la requ�te du client (3 octets)
    uint32_t dtc_mask_record = (data[0] << 16) | (data[1] << 8) | data[2];
    uint8_t found = 0;
    DTC_Record dtc_record;

    // Parcourir tous les DTCs pour trouver celui qui correspond au DTCMaskRecord
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].dtcNumber == dtc_mask_record) {
            dtc_record = stored_dtc_list[i];
            found = 1;
            break;
        }
    }

    // Si aucun DTC correspondant n'est trouv�, envoyer un NRC
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_SEVERITY_INFORMATION_OF_DTC, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Envoyer une r�ponse positive avec les informations de gravit� pour le DTC trouv�
    send_positive_response_read_dtc_information(REPORT_SEVERITY_INFORMATION_OF_DTC, &dtc_record, 1);
}



void report_supported_dtc(uint8_t* data, uint8_t data_length) {
    // Aucune donn�e suppl�mentaire n'est attendue
    if (data_length != 0) {
        send_negative_response_read_dtc_information(REPORT_SUPPORTED_DTC, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t dtc_count = 0;

    // Allouer dynamiquement la m�moire pour stocker les DTCs
    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    // Parcourir les DTCs pour r�cup�rer tous ceux support�s
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].status != 0) {
            dtc_records[dtc_count] = stored_dtc_list[i];
            dtc_count++;
        }
    }

    // Si aucun DTC n'a �t� trouv�, envoyer un NRC
    if (dtc_count == 0) {
        free(dtc_records);
        send_negative_response_read_dtc_information(REPORT_SUPPORTED_DTC, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Envoyer les DTCs support�s
    send_positive_response_read_dtc_information(REPORT_SUPPORTED_DTC, dtc_records, dtc_count);

    // Lib�rer la m�moire
    free(dtc_records);
}


void report_first_test_failed_dtc(uint8_t* data, uint8_t data_length) {
    if (data_length != 0) {
        send_negative_response_read_dtc_information(REPORT_FIRST_TEST_FAILED_DTC, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t found = 0;
    DTC_Record dtc_record;

    // Parcourir les DTCs pour trouver le premier qui a �chou� un test
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].status & DTC_TEST_FAILED) {
            dtc_record = stored_dtc_list[i];
            found = 1;
            break;
        }
    }

    // Si aucun DTC n'a �t� trouv�, envoyer un NRC
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_FIRST_TEST_FAILED_DTC, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Envoyer la r�ponse avec le premier DTC ayant �chou�
    send_positive_response_read_dtc_information(REPORT_FIRST_TEST_FAILED_DTC, &dtc_record, 1);
}



void report_first_confirmed_dtc(uint8_t* data, uint8_t data_length) {
    if (data_length != 0) {
        send_negative_response_read_dtc_information(REPORT_FIRST_CONFIRMED_DTC, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t found = 0;
    DTC_Record dtc_record;

    // Parcourir les DTCs pour trouver le premier confirm�
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].status & DTC_CONFIRMED) {
            dtc_record = stored_dtc_list[i];
            found = 1;
            break;
        }
    }

    // Si aucun DTC confirm� n'est trouv�, envoyer un NRC
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_FIRST_CONFIRMED_DTC, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Envoyer la r�ponse avec le premier DTC confirm�
    send_positive_response_read_dtc_information(REPORT_FIRST_CONFIRMED_DTC, &dtc_record, 1);
}



void report_most_recent_test_failed_dtc(uint8_t* data, uint8_t data_length) {
    if (data_length != 0) {
        send_negative_response_read_dtc_information(REPORT_MOST_RECENT_TEST_FAILED_DTC, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t found = 0;
    DTC_Record* dtc_record = (DTC_Record*)malloc(sizeof(DTC_Record));
    if (dtc_record == NULL) {
        Error_handler();
        return;
    }

    // Parcourir les DTCs pour trouver le plus r�cent ayant �chou�
    for (int8_t i = MAX_DTC_COUNT - 1; i >= 0; i--) {
        if (stored_dtc_list[i].status & DTC_TEST_FAILED) {
            *dtc_record = stored_dtc_list[i];
            found = 1;
            break;
        }
    }

    // Si aucun DTC �chou� n'est trouv�, envoyer un NRC
    if (!found) {
        free(dtc_record);
        send_negative_response_read_dtc_information(REPORT_MOST_RECENT_TEST_FAILED_DTC, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Envoyer la r�ponse avec le DTC le plus r�cent ayant �chou�
    send_positive_response_read_dtc_information(REPORT_MOST_RECENT_TEST_FAILED_DTC, dtc_record, 1);
    free(dtc_record);
}



void report_most_recent_confirmed_dtc(uint8_t* data, uint8_t data_length) {
    if (data_length != 0) {
        send_negative_response_read_dtc_information(REPORT_MOST_RECENT_CONFIRMED_DTC, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t found = 0;
    DTC_Record* dtc_record = (DTC_Record*)malloc(sizeof(DTC_Record));
    if (dtc_record == NULL) {
        Error_handler();
        return;
    }

    // Parcourir les DTCs pour trouver le plus r�cent confirm�
    for (int8_t i = MAX_DTC_COUNT - 1; i >= 0; i--) {
        if (stored_dtc_list[i].status & DTC_CONFIRMED) {
            *dtc_record = stored_dtc_list[i];
            found = 1;
            break;
        }
    }

    // Si aucun DTC confirm� n'est trouv�, envoyer un NRC
    if (!found) {
        free(dtc_record);
        send_negative_response_read_dtc_information(REPORT_MOST_RECENT_CONFIRMED_DTC, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Envoyer la r�ponse avec le DTC le plus r�cent confirm�
    send_positive_response_read_dtc_information(REPORT_MOST_RECENT_CONFIRMED_DTC, dtc_record, 1);
    free(dtc_record);
}




void report_mirror_memory_dtc_by_status_mask(uint8_t* data, uint8_t data_length) {
    if (data_length != 1) { // V�rifier si le masque de statut est fourni
        send_negative_response_read_dtc_information(REPORT_MIRROR_MEMORY_DTC_BY_STATUS_MASK, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t status_mask = data[0]; // Masque de statut envoy� par le client
    uint8_t dtc_count = 0;

    // Allouer dynamiquement de la m�moire pour les DTCs
    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    // Parcourir les DTCs de la m�moire miroir et filtrer par le masque
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if ((mirror_dtc_list[i].status & status_mask) != 0) {
            dtc_records[dtc_count] = mirror_dtc_list[i];
            dtc_count++;
        }
    }

    if (dtc_count == 0) {
        free(dtc_records);
        send_negative_response_read_dtc_information(REPORT_MIRROR_MEMORY_DTC_BY_STATUS_MASK, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Envoyer la r�ponse avec les DTCs de la m�moire miroir
    send_positive_response_read_dtc_information(REPORT_MIRROR_MEMORY_DTC_BY_STATUS_MASK, dtc_records, dtc_count);
    free(dtc_records);
}



void report_mirror_memory_dtc_ext_data_record(uint8_t* data, uint8_t data_length) {
    if (data_length != 4) {
        send_negative_response_read_dtc_information(REPORT_MIRROR_MEMORY_DTC_EXT_DATA_RECORD, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Extraire le num�ro de DTC et d'enregistrement des donn�es �tendues
    uint32_t dtc_mask_record = (data[0] << 16) | (data[1] << 8) | data[2];
    uint8_t ext_data_record_number = data[3];

    uint8_t found = 0;
    DTC_Record* dtc_record = NULL;

    // Parcourir les DTCs dans la m�moire miroir
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (mirror_dtc_list[i].dtcNumber == dtc_mask_record && mirror_dtc_list[i].storedDataRecordNumber == ext_data_record_number) {
            dtc_record = &mirror_dtc_list[i];
            found = 1;
            break;
        }
    }

    // Si aucun DTC n'est trouv�
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_MIRROR_MEMORY_DTC_EXT_DATA_RECORD, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Envoyer une r�ponse positive avec l'enregistrement DTC et les donn�es �tendues
    send_positive_response_read_dtc_information(REPORT_MIRROR_MEMORY_DTC_EXT_DATA_RECORD, dtc_record, 1);
}




void report_number_of_mirror_memory_dtc_by_status_mask(uint8_t* data, uint8_t data_length) {
    if (data_length != 1) {
        send_negative_response_read_dtc_information(REPORT_NUMBER_OF_MIRROR_MEMORY_DTC_BY_STATUS_MASK, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t status_mask = data[0];
    uint16_t dtc_count = 0;

    // Parcourir les DTCs pour compter ceux correspondant au masque de statut
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if ((mirror_dtc_list[i].status & status_mask) != 0) {
            dtc_count++;
        }
    }

    if (dtc_count == 0) {
        send_negative_response_read_dtc_information(REPORT_NUMBER_OF_MIRROR_MEMORY_DTC_BY_STATUS_MASK, NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // Pr�parer la r�ponse
    uint8_t response[4];
    response[0] = get_dtc_status_availability_mask();
    response[1] = get_dtc_format_identifier();
    response[2] = (dtc_count >> 8) & 0xFF;
    response[3] = dtc_count & 0xFF;

    send_can_message(response, 4);
    // send_uart_message(response, 4);
}



void report_number_of_emissions_obd_dtc_by_status_mask(uint8_t* data, uint8_t data_length) {
    if (data_length != 1) {  // Le masque de statut doit �tre d'une longueur de 1 octet
        send_negative_response_read_dtc_information(REPORT_NUMBER_OF_EMISSIONS_OBD_DTC_BY_STATUS_MASK, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t status_mask = data[0];
    uint16_t dtc_count = 0;

    // Parcours de la liste des DTCs
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].isEmissionRelated && (stored_dtc_list[i].status & status_mask) != 0) {
            dtc_count++;
        }
    }

    // Cr�er un enregistrement de DTC pour envoyer la r�ponse
    DTC_Record dtc_record;
    dtc_record.dtcNumber = 0;  // ou un autre num�ro selon la d�finition de la structure
    dtc_record.status = get_dtc_status_availability_mask();
    dtc_record.snapshotDataLength = 4;
    dtc_record.snapshotData[0] = get_dtc_status_availability_mask();
    dtc_record.snapshotData[1] = get_dtc_format_identifier();
    dtc_record.snapshotData[2] = (dtc_count >> 8) & 0xFF;  // Octet de poids fort
    dtc_record.snapshotData[3] = dtc_count & 0xFF;  // Octet de poids faible

    // Envoyer la r�ponse positive
    send_positive_response_read_dtc_information(REPORT_NUMBER_OF_EMISSIONS_OBD_DTC_BY_STATUS_MASK, &dtc_record, 1);
}




void report_emissions_obd_dtc_by_status_mask(uint8_t* data, uint8_t data_length) {
    if (data_length != 1) {
        send_negative_response_read_dtc_information(REPORT_EMISSIONS_OBD_DTC_BY_STATUS_MASK, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t status_mask = data[0];
    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    uint8_t record_count = 0;
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].isEmissionRelated && (stored_dtc_list[i].status & status_mask) != 0) {
            dtc_records[record_count++] = stored_dtc_list[i];
        }
    }

    if (record_count == 0) {
        send_negative_response_read_dtc_information(REPORT_EMISSIONS_OBD_DTC_BY_STATUS_MASK, NRC_CONDITIONS_NOT_CORRECT);
    } else {
        send_positive_response_read_dtc_information(REPORT_EMISSIONS_OBD_DTC_BY_STATUS_MASK, dtc_records, record_count);
    }

    free(dtc_records);
}



void report_dtc_fault_detection_counter(uint8_t* data, uint8_t data_length) {
    if (data_length != 0) {
        send_negative_response_read_dtc_information(REPORT_DTC_FAULT_DETECTION_COUNTER, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    uint8_t record_count = 0;
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].faultDetectionCounter > 0) {
            dtc_records[record_count++] = stored_dtc_list[i];
        }
    }

    if (record_count == 0) {
        send_negative_response_read_dtc_information(REPORT_DTC_FAULT_DETECTION_COUNTER, NRC_CONDITIONS_NOT_CORRECT);
    } else {
        send_positive_response_read_dtc_information(REPORT_DTC_FAULT_DETECTION_COUNTER, dtc_records, record_count);
    }

    free(dtc_records);
}



void report_dtc_with_permanent_status(uint8_t* data, uint8_t data_length) {
    if (data_length != 0) {
        send_negative_response_read_dtc_information(REPORT_DTC_WITH_PERMANENT_STATUS, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    uint8_t record_count = 0;
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].status & DTC_STATUS_PERMANENT) {
            dtc_records[record_count++] = stored_dtc_list[i];
        }
    }

    if (record_count == 0) {
        send_negative_response_read_dtc_information(REPORT_DTC_WITH_PERMANENT_STATUS, NRC_CONDITIONS_NOT_CORRECT);
    } else {
        send_positive_response_read_dtc_information(REPORT_DTC_WITH_PERMANENT_STATUS, dtc_records, record_count);
    }

    free(dtc_records);
}


void report_dtc_ext_data_record_by_record_number(uint8_t* data, uint8_t data_length) {
    if (data_length != 2) {  // Le num�ro d'enregistrement des donn�es �tendues doit avoir 2 octets
        send_negative_response_read_dtc_information(REPORT_DTC_EXT_DATA_RECORD_BY_RECORD_NUMBER, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint16_t record_number = (data[0] << 8) | data[1];
    uint8_t found = 0;
    DTC_Record* dtc_record = NULL;

    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].storedDataRecordNumber == record_number) {
            dtc_record = &stored_dtc_list[i];
            found = 1;
            break;
        }
    }

    if (!found) {
        send_negative_response_read_dtc_information(REPORT_DTC_EXT_DATA_RECORD_BY_RECORD_NUMBER, NRC_REQUEST_OUT_OF_RANGE);
    } else {
        send_positive_response_read_dtc_information(REPORT_DTC_EXT_DATA_RECORD_BY_RECORD_NUMBER, dtc_record, 1);
    }
}


void report_user_def_memory_dtc_by_status_mask(uint8_t* data, uint8_t data_length) {
    if (data_length != 1) {
        send_negative_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_BY_STATUS_MASK, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t status_mask = data[0];
    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));
    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    uint8_t record_count = 0;
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (user_defined_memory_list[i].status & status_mask) {
            dtc_records[record_count++] = user_defined_memory_list[i];
        }
    }

    if (record_count == 0) {
        send_negative_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_BY_STATUS_MASK, NRC_CONDITIONS_NOT_CORRECT);
    } else {
        send_positive_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_BY_STATUS_MASK, dtc_records, record_count);
    }

    free(dtc_records);
}




void report_user_def_memory_dtc_snapshot_record(uint8_t* data, uint8_t data_length) {
    // V�rification de la longueur attendue (DTCMaskRecord + SnapshotRecordNumber)
    if (data_length != 4) {
        send_negative_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_SNAPSHOT_RECORD, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Extraction du DTCMaskRecord et du num�ro de snapshot
    uint32_t dtc_mask_record = (data[0] << 16) | (data[1] << 8) | data[2];
    uint8_t snapshot_record_number = data[3];

    uint8_t found = 0;
    DTC_Record dtc_record;

    // Parcourir la m�moire utilisateur pour trouver le DTC correspondant
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (user_defined_memory_list[i].dtcNumber == dtc_mask_record && user_defined_memory_list[i].snapshotRecordNumber == snapshot_record_number) {
            dtc_record = user_defined_memory_list[i];
            found = 1;
            break;
        }
    }

    // Si aucun enregistrement n'est trouv�
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_SNAPSHOT_RECORD, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Envoyer la r�ponse positive avec l'enregistrement snapshot
    send_positive_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_SNAPSHOT_RECORD, &dtc_record, 1);
}




void report_user_def_memory_dtc_ext_data_record(uint8_t* data, uint8_t data_length) {
    // V�rification de la longueur des donn�es re�ues
    if (data_length != 4) {
        send_negative_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_EXT_DATA_RECORD, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Extraction du DTCMaskRecord et du num�ro de l'enregistrement de donn�es �tendues
    uint32_t dtc_mask_record = (data[0] << 16) | (data[1] << 8) | data[2];
    uint8_t ext_data_record_number = data[3];

    uint8_t found = 0;
    DTC_Record dtc_record;

    // Parcourir la m�moire utilisateur pour trouver le DTC correspondant
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (user_defined_memory_list[i].dtcNumber == dtc_mask_record && user_defined_memory_list[i].storedDataRecordNumber == ext_data_record_number) {
            dtc_record = user_defined_memory_list[i];
            found = 1;
            break;
        }
    }

    // Si aucun enregistrement n'est trouv�
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_EXT_DATA_RECORD, NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Envoyer la r�ponse positive avec les donn�es �tendues du DTC
    send_positive_response_read_dtc_information(REPORT_USER_DEF_MEMORY_DTC_EXT_DATA_RECORD, &dtc_record, 1);
}




void report_wwh_obd_dtc_by_mask_record(uint8_t* data, uint8_t data_length) {
    if (data_length != 1) {  // Le masque de statut doit �tre d'une longueur de 1 octet
        send_negative_response_read_dtc_information(REPORT_WWH_OBD_DTC_BY_MASK_RECORD, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t status_mask = data[0];
    uint8_t found = 0;
    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));

    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    uint8_t record_count = 0;

    // Parcourir la m�moire pour trouver les DTCs WWH-OBD
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].isEmissionRelated && (stored_dtc_list[i].status & status_mask) != 0) {
            dtc_records[record_count++] = stored_dtc_list[i];
            found = 1;
        }
    }

    // Si aucun DTC n'est trouv�
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_WWH_OBD_DTC_BY_MASK_RECORD, NRC_CONDITIONS_NOT_CORRECT);
    } else {
        send_positive_response_read_dtc_information(REPORT_WWH_OBD_DTC_BY_MASK_RECORD, dtc_records, record_count);
    }

    free(dtc_records);
}



void report_wwh_obd_dtc_with_permanent_status(uint8_t* data, uint8_t data_length) {
    if (data_length != 0) {
        send_negative_response_read_dtc_information(REPORT_WWH_OBD_DTC_WITH_PERMANENT_STATUS, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    uint8_t found = 0;
    DTC_Record* dtc_records = (DTC_Record*)malloc(MAX_DTC_COUNT * sizeof(DTC_Record));

    if (dtc_records == NULL) {
        Error_handler();
        return;
    }

    uint8_t record_count = 0;

    // Parcourir la m�moire pour trouver les DTCs WWH-OBD avec statut permanent
    for (uint8_t i = 0; i < MAX_DTC_COUNT; i++) {
        if (stored_dtc_list[i].status & DTC_STATUS_PERMANENT) {
            dtc_records[record_count++] = stored_dtc_list[i];
            found = 1;
        }
    }

    // Si aucun DTC n'est trouv�
    if (!found) {
        send_negative_response_read_dtc_information(REPORT_WWH_OBD_DTC_WITH_PERMANENT_STATUS, NRC_CONDITIONS_NOT_CORRECT);
    } else {
        send_positive_response_read_dtc_information(REPORT_WWH_OBD_DTC_WITH_PERMANENT_STATUS, dtc_records, record_count);
    }

    free(dtc_records);
}



uint8_t get_dtc_status_availability_mask() {
    // Retourne un masque de statut DTC selon tes besoins
    return 0xFF; // Exemple : Tous les statuts sont disponibles
}
uint8_t get_dtc_format_identifier() {
    // Retourne un identifiant de format DTC (ex : SAE J2012, ISO, etc.)
    return 0x01; // Exemple
}



