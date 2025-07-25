/*
 * a.c
 *
 *  Created on: Jul 9, 2025
 *      Author: suneel.kataraki
 */
#include "InputOutput_Control_functional_unit.h"
#include "uds_services.h"
#include <string.h>
#include"main.h"


/************************************************InputOutputControlByIdentifier*************************************************/

// Fonction pour g�rer la commande InputOutputControlByIdentifier
void uds_input_output_control_by_identifier(IOControlRequest_t *request, IOControlResponse_t *response) {
    // V�rification de la longueur minimale du message
    if (sizeof(*request) < (sizeof(request->SID) + sizeof(request->dataIdentifier) + sizeof(request->controlOptionRecord))) {
        send_negative_response_input_output_control_by_identifier(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // V�rification de la prise en charge de l'Identifiant de Donn�es (DID)
    if (!is_data_identifier_supported_for_write(request->dataIdentifier)) {
        send_negative_response_input_output_control_by_identifier(NRC_REQUEST_OUT_OF_RANGE);
        return;
    }

    // Traitement des options de contr�le
    switch (request->controlOptionRecord) {
        case IOCP_SHORT_TERM_ADJUSTMENT:
            // Appliquer les �tats de contr�le
            // Ici, vous devez ajouter votre logique pour contr�ler l'ECU
            // Exemple : write_data_to_identifier(request->dataIdentifier, request->controlState, sizeof(request->controlState));
            break;

        case IOCP_RETURN_CONTROL_TO_ECU:
            // Logique pour retourner le contr�le � l'ECU
            // Exemple : return_control_to_ecu(request->dataIdentifier);
            break;

        case IOCP_FREEZE_CURRENT_STATE:
            // Logique pour geler l'�tat actuel
            // Exemple : freeze_current_state(request->dataIdentifier);
            break;

        default:
            send_negative_response_input_output_control_by_identifier(NRC_CONDITIONS_NOT_CORRECT);
            return;
    }

    // Pr�parer la r�ponse positive
    response->SID = UDS_RESPONSE_INPUT_OUTPUT_CONTROL;
    response->dataIdentifier = request->dataIdentifier;

    // Remplir le tableau de statut de contr�le avec les valeurs actuelles ou d�sir�es
    memcpy(response->controlStatusRecord, request->controlState, sizeof(request->controlState));

    // Envoyer la r�ponse positive
    send_positive_response_input_output_control_by_identifier(request->dataIdentifier, request->controlOptionRecord, response->controlStatusRecord);
}

// Fonction pour envoyer une r�ponse positive
void send_positive_response_input_output_control_by_identifier(uint16_t dataIdentifier, uint8_t controlOptionRecord, uint8_t *controlStatusRecord) {
    IOControlResponse_t response;

    response.SID = UDS_RESPONSE_INPUT_OUTPUT_CONTROL;
    response.dataIdentifier = dataIdentifier;

    // Copie des �tats de contr�le dans la r�ponse
    memcpy(response.controlStatusRecord, controlStatusRecord, sizeof(response.controlStatusRecord));

    // Envoi du message via CAN
    send_can_message((uint8_t*)&response, sizeof(response));
    // send_uart_message((uint8_t*)&response, sizeof(response));
}


// Fonction pour envoyer une r�ponse n�gative
void send_negative_response_input_output_control_by_identifier(uint8_t nrc) {
    // Envoi d'un message d'erreur par CAN
    // Exemple : send_can_message(&nrc, sizeof(nrc));
    uint8_t response[2];
    response[0] = UDS_RESPONSE_INPUT_OUTPUT_CONTROL; // SID de r�ponse
    response[1] = nrc; // NRC

    send_can_message(response, sizeof(response));
    // send_uart_message(response, sizeof(response));
}

// Fonction pour g�rer la r�ponse au service
void handle_input_output_control_response(IOControlResponse_t *response) {
    // Logique pour traiter la r�ponse si n�cessaire
    // Exemple : afficher l'�tat de contr�le re�u
}



