/*
 * a.c
 *
 *  Created on: Jul 9, 2025
 *      Author: suneel.kataraki
 */


/*
 * Upload_Download_functional_unit.c
 *
 *  Created on: 21 oct. 2024
 *      Author: PC
 */
#include "uds_services.c"
#include "Upload_Download_functional_unit.h"
/*******************************************************Request_download******************************************************/
// Fonction pour traiter la demande de t�l�chargement
void uds_request_download(RequestDownload_t *request) {
    // 1. V�rification de la longueur minimale
    if (sizeof(*request) < sizeof(request->dataFormatIdentifier) + sizeof(request->addressAndLengthFormatIdentifier) + 4) {
        send_negative_response_request_download(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // 2. V�rification de l'identifiant de format de donn�es
    if (request->dataFormatIdentifier != 0x00) { // Exemple de v�rification
        send_negative_response_request_download(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // 3. V�rification de l'identifiant de format d'adresse et de longueur
    if (!is_memory_address_valid(request->memoryAddress) || !is_memory_size_valid(request->memorySize)) {
        send_negative_response_request_download(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // 4. V�rification de la s�curit� (si applicable)
    if (is_security_active()) { // Impl�mentez is_security_active()
        send_negative_response_request_download(NRC_SECURITY_ACCESS_DENIED);
        return;
    }

    // Pr�parer la r�ponse positive
    ResponseDownload_t response;
    response.lengthFormatIdentifier = 0x74; // Identifiant de r�ponse
    response.maxNumberOfBlockLength[0] = 0x00; // Remplacez par la logique appropri�e
    response.maxNumberOfBlockLength[1] = 0xFF; // Remplacez par la logique appropri�e

    // Envoyer la r�ponse positive
    send_positive_response_request_download(&response);
}

// Fonction pour envoyer une r�ponse positive
void send_positive_response_request_download(ResponseDownload_t *response) {
    // Envoi du message via CAN
    send_can_message((uint8_t *)response, sizeof(ResponseDownload_t));
    // send_uart_message((uint8_t *)response, sizeof(ResponseDownload_t));
}

// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_request_download(uint8_t nrc) {
    uint8_t response[2];
    response[0] = UDS_RESPONSE_REQUEST_DOWNLOAD; // SID de r�ponse
    response[1] = nrc; // NRC
    send_can_message(response, sizeof(response));
    //send_uart_message(response, sizeof(response));
}

// Fonction pour v�rifier la validit� de l'adresse m�moire
bool is_memory_address_valid(uint8_t *address) {
    // Impl�mentez votre logique pour v�rifier la validit� de l'adresse
    return true; // Remplacez par la logique appropri�e
}

// Fonction pour v�rifier la validit� de la taille m�moire
bool is_memory_size_valid(uint8_t *size) {
    // Impl�mentez votre logique pour v�rifier la validit� de la taille
    return true; // Remplacez par la logique appropri�e
}
/*******************************************************RequestUpload******************************************************/

// Fonction pour g�rer la requ�te RequestUpload
void uds_request_upload(RequestUpload_t *request) {
    // 1. V�rification de la longueur du message
    if (sizeof(*request) < sizeof(request->dataFormatIdentifier) + sizeof(request->addressAndLengthFormatIdentifier)) {
        send_negative_response_upload(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // 2. V�rification de l'identifiant et du format de donn�es
    if (request->dataFormatIdentifier != 0x35) { // Exemple de validation
        send_negative_response_upload(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // 3. V�rification de la s�curit�
    if (is_security_active()) {
        send_negative_response_upload(NRC_SECURITY_ACCESS_DENIED);
        return;
    }

    // 4. Validation des donn�es dans memoryAddress et memorySize
    // Impl�mentez votre logique pour v�rifier si l'adresse et la taille sont valides

    // 5. Envoyer une r�ponse positive
    uint8_t maxNumberOfBlockLength[2] = {0x00, 0xFF}; // Exemple
    send_positive_response_upload(maxNumberOfBlockLength);
}

// Fonction pour envoyer une r�ponse positive
void send_positive_response_upload(uint8_t *maxNumberOfBlockLength) {
    ResponseUpload_t response;
    response.lengthFormatIdentifier = 0x75; // LFID
    response.maxNumberOfBlockLength[0] = maxNumberOfBlockLength[0];
    response.maxNumberOfBlockLength[1] = maxNumberOfBlockLength[1];

    // Envoyer le message via le protocole CAN ou autre m�thode appropri�e
    send_can_message((uint8_t*)&response, sizeof(response));
    //send_uart_message((uint8_t*)&response, sizeof(response));
}

// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_upload(uint8_t nrc) {
    uint8_t response[2];
    response[0] = 0x35; // SID de r�ponse
    response[1] = nrc;

    // Envoyer le message d'erreur
    send_can_message(response, sizeof(response));
    //send_uart_message(response, sizeof(response));
}

// Exemple de fonction pour v�rifier si le niveau de s�curit� est actif
bool is_security_active(void) {
    // Logique pour v�rifier si la s�curit� est active
    return false; // Remplacez par votre logique
}

// Fonction pour valider la requ�te
bool validate_request_upload(RequestUpload_t *request) {
    // Impl�mentez la logique de validation
    return true; // Remplacez par votre logique
}

/*******************************************************TransferData******************************************************/
#include "uds_services.h"

// Fonction pour g�rer la requ�te TransferData
void uds_transfer_data(RequestTransferData_t *request) {
    // 1. V�rification de la longueur du message
    if (sizeof(*request) < sizeof(request->blockSequenceCounter)) {
        send_negative_response_transfer_data(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // 2. V�rification si une requ�te de t�l�chargement ou d'upload est active
    if (!is_request_download_active() && !is_request_upload_active()) {
        send_negative_response_transfer_data(NRC_REQUEST_SEQUENCE_ERROR);
        return;
    }

    // 3. V�rification de la validit� du blockSequenceCounter
    static uint8_t lastBlockSequenceCounter = 0;
    if (request->blockSequenceCounter != (lastBlockSequenceCounter + 1) % 256) {
        send_negative_response_transfer_data(NRC_WRONG_BLOCK_SEQUENCE_COUNTER);
        return;
    }

    // 4. Traitement des donn�es transf�r�es
    // Si c'est un t�l�chargement, �crivez les donn�es dans la m�moire
    // Si c'est un upload, lisez les donn�es � partir de la m�moire
    // Exemple (� adapter selon vos besoins) :
    // write_data_to_memory(request->transferRequestParameterRecord);

    // 5. Mettez � jour le dernier blockSequenceCounter utilis�
    lastBlockSequenceCounter = request->blockSequenceCounter;

    // 6. Envoyer une r�ponse positive
    send_positive_response_transfer_data(request->blockSequenceCounter);
}

// Fonction pour envoyer une r�ponse positive
#include <string.h> // Inclure pour memcpy

void send_positive_response_transfer_data(uint8_t blockSequenceCounter) {
    ResponseTransferData_t response;
    response.blockSequenceCounter = blockSequenceCounter;

    // Remplir response.transferResponseParameterRecord si n�cessaire
    // Exemple :
    // response.transferResponseParameterRecord[0] = ...;

    // Calculez la taille totale du message � envoyer
    size_t responseSize = sizeof(response.blockSequenceCounter) +
                          sizeof(response.transferResponseParameterRecord); // Ajoutez la taille de tous les membres

    // Envoyer le message via le protocole CAN ou autre m�thode appropri�e
    send_can_message((uint8_t*)&response, responseSize);
    // send_uart_message((uint8_t*)&response, responseSize);
}

// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_transfer_data(uint8_t nrc) {
    uint8_t response[2];
    response[0] = 0x36; // SID de r�ponse
    response[1] = nrc;

    // Envoyer le message d'erreur
    send_can_message(response, sizeof(response));
    //send_uart_message(response, sizeof(response));
}

// Exemple de v�rification d'activit� des requ�tes
bool is_request_download_active(void) {
    // Logique pour v�rifier si une requ�te de t�l�chargement est active
    return false; // Remplacez par votre logique
}

bool is_request_upload_active(void) {
    // Logique pour v�rifier si une requ�te d'upload est active
    return false; // Remplacez par votre logique
}

// Fonction pour valider la requ�te de transfert de donn�es
bool validate_transfer_data_request(RequestTransferData_t *request) {
    // Impl�mentez la logique de validation
    return true; // Remplacez par votre logique
}

/*******************************************************RequestTransferExit*************************************************/
// Fonction pour g�rer la requ�te RequestTransferExit
void uds_request_transfer_exit(RequestTransferExit_t *request, ResponseTransferExit_t *response) {
    // 1. V�rification de la longueur du message
    if (sizeof(*request) < sizeof(request->transferRequestParameterRecord)) {
        send_negative_response_transfer_exit(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // 2. V�rification de l'�tat de la s�quence de demande
    if (!is_transfer_in_progress()) {
        send_negative_response_transfer_exit(NRC_REQUEST_SEQUENCE_ERROR);
        return;
    }

    // 3. Validation des donn�es dans transferRequestParameterRecord
    if (!validate_transfer_request_parameters(request)) {
        send_negative_response_transfer_exit(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // 4. Logique pour finaliser la requ�te de transfert
    // (Ajoutez ici la logique n�cessaire pour finaliser le transfert de donn�es)

    // 5. Pr�paration de la r�ponse positive
    memset(response, 0, sizeof(ResponseTransferExit_t)); // R�initialiser la r�ponse
    // Remplir response.transferResponseParameterRecord si n�cessaire
    // Exemple : response.transferResponseParameterRecord[0] = ...;

    // Envoyer la r�ponse positive
    send_positive_response_transfer_exit(response);
}

// Fonction pour envoyer une r�ponse positive
void send_positive_response_transfer_exit(ResponseTransferExit_t *response) {
    // Envoi du message via le protocole CAN ou autre m�thode appropri�e
    size_t response_size = sizeof(ResponseTransferExit_t); // Taille de la r�ponse
    send_can_message((uint8_t*)response, response_size);
}

// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_transfer_exit(uint8_t nrc) {
    // Envoi d'un message d'erreur par CAN
    uint8_t response[2];
    response[0] = UDS_RESPONSE_TRANSFER_EXIT; // SID de r�ponse
    response[1] = nrc; // NRC

    send_can_message(response, sizeof(response));
    //send_uart_message(response, sizeof(response));
}

// Fonction pour v�rifier si un transfert est en cours
bool is_transfer_in_progress(void) {
    // Impl�mentez votre logique pour v�rifier si un transfert est actif
    return true; // Remplacez ceci par la logique appropri�e
}

// Fonction pour valider les param�tres de la requ�te de transfert
bool validate_transfer_request_parameters(RequestTransferExit_t *request) {
    // Impl�mentez votre logique pour valider les param�tres
    return true; // Remplacez ceci par la logique appropri�e
}

/*******************************************************RequestFileTransfer*************************************************/
// Fonction pour traiter la requ�te RequestFileTransfer
void uds_request_file_transfer(RequestFileTransfer_t *request) {
    // 1. V�rification de la longueur du message
    if (request->filePathAndNameLength > sizeof(request->filePathAndName)) {
        send_negative_response_file_transfer(0x13); // Longueur incorrecte
        return;
    }

    // 2. V�rification du mode d'op�ration
    if (request->modeOfOperation < 0x01 || request->modeOfOperation > 0x05) {
        send_negative_response_file_transfer(0x31); // Mode hors plage
        return;
    }

    // 3. V�rifications suppl�mentaires selon le mode d'op�ration
    // (Ajoutez ici des v�rifications sp�cifiques si n�cessaire)

    // 4. Envoi de la r�ponse positive
    ResponseFileTransfer_t response;
    response.SID = 0x78; // Identifiant de r�ponse
    response.modeOfOperation = request->modeOfOperation;
    response.lengthFormatIdentifier = 0x00; // � d�finir selon le contexte
    response.maxNumberOfBlockLength[0] = 0xFF; // Exemple de longueur maximale
    response.maxNumberOfBlockLength[1] = 0xFF; // Exemple de longueur maximale
    response.dataFormatIdentifier = request->dataFormatIdentifier;

    // Remplir les longueurs de fichier si n�cessaire
    // Exemple : response.fileSizeOrDirInfoParameterLength = ...;

    // Appeler la fonction pour envoyer la r�ponse positive
    send_positive_response_file_transfer(&response);
}

// Fonction pour envoyer une r�ponse positive
void send_positive_response_file_transfer(ResponseFileTransfer_t *response) {
    // Envoi du message via le protocole CAN ou autre m�thode appropri�e
    send_can_message((uint8_t*)response, sizeof(ResponseFileTransfer_t));
    //send_uart_message((uint8_t*)response, sizeof(ResponseFileTransfer_t));
}


// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_file_transfer(uint8_t nrc) {
    uint8_t response[2];
    response[0] = 0x78; // Identifiant de r�ponse
    response[1] = nrc; // NRC

    // Envoi d'un message d'erreur par CAN
    send_can_message(response, sizeof(response));
    // send_uart_message(response, sizeof(response));
}

