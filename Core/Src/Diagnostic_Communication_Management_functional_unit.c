/*
 * b.c
 *
 *  Created on: Jul 9, 2025
 *      Author: suneel.kataraki
 */

#include "uds_services.c"
#include "Diagnostic_Communication_Management_functional_unit.h"
#include "main.h"
#include "uds_services.h"


/****************************************Diagnostic Session Control************************************************************/
// Declaration of the UDS Session structure
UDS_Session uds_session = {
		.current_session = UDS_SESSION_DEFAULT,
		.security_access_granted = false,
		.s3_timeout_active = false,
        .s3_timer_counter = 50000};

TimingParameters timing_para; //  object for p2_server_max, p2_star_server_max


/**
 * @brief Diagnostic Session Control
 * @param session_type : Session type for diagnostic (Default, Programming, Extended, Safety)
 */
void uds_diagnostic_session_control(uint8_t session_type, uint8_t len) {
    uint8_t message_length = len; // Example of UDS message length

    if (!is_service_allowed(UDS_DIAGNOSTIC_SESSION_CONTROL)) {
            send_negative_response_diagnostic_control(UDS_DIAGNOSTIC_SESSION_CONTROL, NRC_CONDITIONS_NOT_CORRECT);
            return;
        }
    if (message_length != 2) {
        send_negative_response_diagnostic_control(UDS_DIAGNOSTIC_SESSION_CONTROL, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    switch (session_type) {
        case UDS_SESSION_DEFAULT:
            if (uds_session.current_session == UDS_SESSION_DEFAULT)
            {
            	//reseting the s3 timer to 0
            	reset_s3_timer();
            	set_p2_p2_star_server_timer();
            	// Reset the session to default
                reset_default_session();
                send_positive_response_diagnostic_control(UDS_DIAGNOSTIC_SESSION_CONTROL);
            }
            else
            {
            	//reseting the s3 timer to 0
            	reset_s3_timer();
            	set_p2_p2_star_server_timer();
            	// Switch from non-default session to default session
                reset_events();
                deactivate_extended_services();
                uds_session.current_session = UDS_SESSION_DEFAULT;
                send_positive_response_diagnostic_control(UDS_DIAGNOSTIC_SESSION_CONTROL);
            }
            break;

        case UDS_SESSION_PROGRAMMING:
        	//reseting the s3 timer to 0
        	reset_s3_timer();
        	set_p2_p2_star_server_timer();
        	// Switch to the programming session
            reset_events();
            uds_session.current_session = UDS_SESSION_PROGRAMMING;
            send_positive_response_diagnostic_control(UDS_DIAGNOSTIC_SESSION_CONTROL);
            break;

        case UDS_SESSION_EXTENDED_DIAGNOSTIC:
        	//reseting the s3 timer to 0
        	reset_s3_timer();
        	set_p2_p2_star_server_timer();

        	// Passing the extended diagnostic session
            reset_events();
            uds_session.current_session = UDS_SESSION_EXTENDED_DIAGNOSTIC;
            send_positive_response_diagnostic_control(UDS_DIAGNOSTIC_SESSION_CONTROL);
            break;

        case UDS_SESSION_SAFETY_SYSTEM:
        	reset_s3_timer();
        	set_p2_p2_star_server_timer();

        	// Switch to the security session.
            reset_events();
            uds_session.current_session = UDS_SESSION_SAFETY_SYSTEM;
            send_positive_response_diagnostic_control(UDS_DIAGNOSTIC_SESSION_CONTROL);
            break;

        default:
        	// Negative response for unsupported subfunction
            send_negative_response_diagnostic_control(UDS_DIAGNOSTIC_SESSION_CONTROL, NRC_SUB_FUNCTION_NOT_SUPPORTED);
            break;
    }
}


/**
 * @brief Reset session to default
 */
void reset_default_session() {
	// Resets the session settings to default
    uds_session.security_access_granted = false;
    // Reset events and other specific parameters
    //reset_events();
    deactivate_extended_services();
}

/**
 * @brief Initialization of diagnostic session server timer
 */
void set_p2_p2_star_server_timer()
{
	//initializing the p2 and p2* server timer
	timing_para.p2_server_max = 50;
	timing_para.p2_star_server_max = 500;
}
/**
 * @brief when the session switch from one session to other session reset the s3 timer
 */
void reset_s3_timer()
{
	//resetting s3 timer
    uds_session.s3_timer = 0;
    uds_session.s3_timeout_active = true;
}

/**
 * @brief Reset session to default when session timer exceeds the s3 timer
 */
void UDS_Timer_1ms_Tick(void)
{
    // Handle S3
    if (uds_session.s3_timeout_active)
    {
        uds_session.s3_timer++;
        if (uds_session.s3_timer >= uds_session.s3_timer_counter)
        {
            uds_session.current_session = UDS_SESSION_DEFAULT;
            uds_session.s3_timeout_active = false;
        }
    }

    // Similarly, call P2 / P2* tick functions if enabled
}


/**
 * @brief Resets active events (eg: ResponseOnEvent)
 */
void reset_events()
{
	// Stops all active events triggered in non-default sessions


}

/**
 * @brief Disables unsupported services in the default session
 */
void deactivate_extended_services() {
	// Disable extended services like CommunicationControl, ResponseOnEvent, etc.
}

/**
 * @brief Checks if a service is allowed in the current session
 * @param service_id: UDS service identifier
 * @return true if the service is authorized, false otherwise
 */
bool is_service_allowed(uint8_t service_id) {
    switch (service_id) {
        case UDS_DIAGNOSTIC_SESSION_CONTROL:
        case UDS_ECU_RESET:
        case UDS_TESTER_PRESENT:
        case UDS_CLEAR_DIAGNOSTIC_INFORMATION:
        case UDS_READ_DTC_INFORMATION:
        	// These services are always allowed in both sessions
            return true;

        case UDS_SECURITY_ACCESS:
        case UDS_COMMUNICATION_CONTROL:
        case UDS_ACCESS_TIMING_PARAMETER:
        case UDS_SECURED_DATA_TRANSMISSION:
        case UDS_CONTROL_DTC_SETTING:
        case UDS_RESPONSE_ON_EVENT:
        case UDS_LINK_CONTROL:
        case UDS_READ_MEMORY_BY_ADDRESS:
        case UDS_WRITE_MEMORY_BY_ADDRESS:
        case UDS_REQUEST_DOWNLOAD:
        case UDS_REQUEST_UPLOAD:
        case UDS_TRANSFER_DATA:
        case UDS_REQUEST_TRANSFER_EXIT:
        case UDS_REQUEST_FILE_TRANSFER:
        	// These services are only allowed in non-default sessions
            return uds_session.current_session != UDS_SESSION_DEFAULT;


        case UDS_READ_DATA_BY_IDENTIFIER:
        case UDS_WRITE_DATA_BY_IDENTIFIER:
        case UDS_READ_SCALING_DATA_BY_IDENTIFIER:
        case UDS_DYNAMICAL_DEFINE_DATA_IDENTIFIER:
        	// These services require security access, so they are not allowed in session by default
            return uds_session.current_session != UDS_SESSION_DEFAULT && uds_session.security_access_granted;

        case UDS_ROUTINE_CONTROL:
        	// RoutineControl also requires security access and a non-default session
            return uds_session.current_session != UDS_SESSION_DEFAULT && uds_session.security_access_granted;

        default:
        	// For other services not listed
            return false;
    }
}


/**
 * @brief Sends a positive response to the client
 * @param service_id: Identifier of the service for which the response is sent
 */
void send_positive_response_diagnostic_control(uint8_t service_id) {
   uint8_t response[8] = {0};
   response[0]=0x06;
   // SID response to indicate success (add 0x40 to service ID)
    response[1] = service_id + 0x40; // For example, for DiagnosticSessionControl, it will be 0x50

    // Sub-function : session type response current (default, extended, etc.)
    response[2] = uds_session.current_session;

    // Session Parameter Record: additional data (P2Server_max and P2*Server_max for example)
    // Let's simulate values for these timings (in ms)

    response[3] = (timing_para.p2_server_max >> 8) & 0xFF;        // P2Server_max high byte
    response[4] = (timing_para.p2_server_max) & 0xFF;             // P2Server_max low byte (50 ms par exemple)
    response[5] = (timing_para.p2_star_server_max >> 8) & 0xFF;   // P2*Server_max high byte
    response[6] = (timing_para.p2_star_server_max) & 0xFF;        // P2*Server_max low byte (500 ms par exemple)
    // Length of response data (6 bytes in this case)

    uds_session.suppress = suppress;

    if(!uds_session.suppress)
    {
    	send_can_message(response, 7);
    }
}

/**
 * @brief Envoie une r�ponse n�gative au client
 * @param service_id : Identifiant du service pour lequel la r�ponse est envoy�e
 * @param nrc : Code de r�ponse n�gative (NRC)
 */
void send_negative_response_diagnostic_control(uint8_t service_id, uint8_t nrc) {
    uint8_t response[4] = {0};
    response[0]=0x04;
    response[1] = UDS_NEGATIVE_RESPONSE; // R�ponse n�gative g�n�rique
    response[2] = service_id; // Service concern�
    response[3] = nrc; // Code NRC (Negative Response Code)

    send_can_message(response, 4);
}

/**********************************************ECU RESET***********************************************************************/
/**
 * @brief ECU Reset Service
 * @param resetType : Type of reset requested by the client (e.g., hard reset, soft reset, etc.)
 */
void uds_ecu_reset(uint8_t resetType, uint8_t len) {
	// Verification of the validity of the resetType
    if (resetType > UDS_RESET_TYPE_DISABLE_RAPID_POWER_SHUTDOWN) {
        send_negative_response_ecu_reset(UDS_ECU_RESET, NRC_SUB_FUNCTION_NOT_SUPPORTED);
        return;
    }

    // Build the positive response (SID = 0x51)
    uint8_t response[4] = {0x04, 0x51, resetType, 0xFF}; // 0xFF for "powerDownTime" if applicable
    send_can_message(response, 4);  // Send the response before executing the reset

    // Execution of reset according to resetType
    switch (resetType) {
        case UDS_RESET_TYPE_HARD_RESET:
            hard_reset();  // Function that performs a hard reset
            break;
        case UDS_RESET_TYPE_SOFT_RESET:
            soft_reset();  // Function that performs a soft reset
            break;
        case UDS_RESET_TYPE_ENABLE_RAPID_POWER_SHUTDOWN:
        	// Enable the fast power shutdown function
            enable_rapid_power_shutdown();
            break;
        case UDS_RESET_TYPE_DISABLE_RAPID_POWER_SHUTDOWN:
        	// Disable the fast power off function
            disable_rapid_power_shutdown();
            break;
        default:
        	// In case of unsupported reset type
            send_negative_response_ecu_reset(UDS_ECU_RESET, NRC_SUB_FUNCTION_NOT_SUPPORTED);
            return;
    }

    // After reset, return to the default session
    uds_session.current_session = UDS_SESSION_DEFAULT;
}

void hard_reset() {
	// Using the NVIC function to trigger a hard reset
    NVIC_SystemReset();
}
void key_on_off_reset()
{

}
void soft_reset() {
	// Example of a soft reset that could restart the application without resetting the hardware registers
	// You can define your own logic here to reset only the necessary parts
	// for example, reset some devices or restart tasks without resetting the entire system.

	// Restart the main loop or reset some devices
}
void enable_rapid_power_shutdown() {
	// Enter low power mode
	// Example with the STM32 Stop mode
    HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}
void disable_rapid_power_shutdown() {

}


/**
 * @brief Sends a positive response for the ECUReset service
 * @param resetType : Type of reset that was requested by the client
 * @param powerDownTime : Time in seconds the ECU will stay in power-down mode (only for rapid power shutdown)
 */
void send_positive_response_ecu_reset(uint8_t resetType, uint8_t powerDownTime) {
    uint8_t response[3] = {0};

    // Response SID (0x51 for ECUReset Response)
    response[0] = 0x51;

    // Sub-function echoing the resetType (received from the client request)
    response[1] = resetType;

    // PowerDownTime is only included when the resetType is 'Enable Rapid Power Shutdown' (0x04)
    if (resetType == UDS_RESET_TYPE_ENABLE_RAPID_POWER_SHUTDOWN) {
        response[2] = powerDownTime;  // 0x00 to 0xFE for seconds, 0xFF for not available
        send_can_message(response, 3); // Send a message with 3 bytes
    } else {
        // If powerDownTime is not applicable, only send the first two bytes
        send_can_message(response, 2); // Send a message with 2 bytes
    }
}

/**
 * @brief Envoie une r�ponse n�gative pour le service ECUReset
 * @param service_id : Identifiant du service ECUReset (0x11)
 * @param nrc : Code de r�ponse n�gative (NRC)
 */
void send_negative_response_ecu_reset(uint8_t service_id, uint8_t nrc) {
    uint8_t response[4] = {0};

    // Remplir le message de r�ponse n�gative
    response[0]=0x03;
    response[1] = UDS_NEGATIVE_RESPONSE;         // R�ponse n�gative g�n�rique
    response[2] = service_id;   // Service concern� (ECUReset 0x11)
    response[3] = nrc;          // NRC sp�cifique

    // Traitement des diff�rents NRC en fonction des conditions
    switch (nrc) {
        case NRC_SUB_FUNCTION_NOT_SUPPORTED:
            // Code 0x12 : Sous-fonction non prise en charge
            response[2] = NRC_SUB_FUNCTION_NOT_SUPPORTED;
            break;
        case NRC_INCORRECT_MESSAGE_LENGTH:
            // Code 0x13 : Longueur ou format du message incorrect
            response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
            break;
        case NRC_CONDITIONS_NOT_CORRECT:
            // Code 0x22 : Conditions non respect�es pour l'ECU Reset
            response[2] = NRC_CONDITIONS_NOT_CORRECT;
            break;
        case NRC_SECURITY_ACCESS_DENIED:
            // Code 0x33 : Acc�s de s�curit� refus�
            response[2] = NRC_SECURITY_ACCESS_DENIED;
            break;
        default:
            // Autres codes de r�ponse n�gative non d�finis
            response[2] = 0x7F; // Code g�n�rique d'erreur si non reconnu
            break;
    }

    // Envoyer la r�ponse CAN avec le code NRC appropri�
    send_can_message(response, 4);
}

/**********************************************Security access***********************************************************************/
/**
 * @brief Service Security Access (0x27)
 * @param sub_function : Sous-fonction (0x01 pour demande de seed, 0x02 pour v�rification de cl�)
 * @param data : Pointeur vers les donn�es envoy�es (key ou autres param�tres)
 * @param data_length : Longueur des donn�es envoy�es
 */
// D�finition des variables de seed et key
uint8_t security_seed_lvl1[2] = {0x36,0x57}; // Seed pr d fini pour niveau 1
uint8_t security_key_lvl1[2] = {0xC9,0xA9};  // Cl  pr d finie pour le seed de niveau 1
uint8_t security_seed_lvl2[2] = {0x5A,0x9B}; // Seed pr d fini pour niveau 2
uint8_t security_key_lvl2[2]= {0xA5,0x65};  // Cl  pr d finie pour le seed de niveau 2

void uds_security_access(uint8_t sub_function, uint8_t *data, uint8_t data_length) {
    switch (sub_function) {
        case UDS_SECURITY_ACCESS_REQUEST_SEED_LVL1:
            // Demande de seed pour le niveau 1
            send_seed(UDS_SECURITY_ACCESS_REQUEST_SEED_LVL1);
            //send_positive_response_security_access(UDS_SECURITY_ACCESS_REQUEST_SEED_LVL1);
            break;

        case UDS_SECURITY_ACCESS_SEND_KEY_LVL1:
            // V rifier la longueur des donn es envoy es
            if (data_length != 4) {
                send_negative_response_security_access(sub_function, NRC_INCORRECT_MESSAGE_LENGTH);
                return;
            }
            // V rifier la cl  envoy e
            if (verify_key(UDS_SECURITY_ACCESS_SEND_KEY_LVL1, data)) {
                send_positive_response_security_access(UDS_SECURITY_ACCESS_SEND_KEY_LVL1);
            } else {
                send_negative_response_security_access(sub_function, NRC_INVALID_KEY);
            }
            break;

        case UDS_SECURITY_ACCESS_REQUEST_SEED_LVL2:
            // Demande de seed pour le niveau 2
            send_seed(UDS_SECURITY_ACCESS_REQUEST_SEED_LVL2);
            //send_positive_response_security_access(UDS_SECURITY_ACCESS_REQUEST_SEED_LVL2);
            break;

        case UDS_SECURITY_ACCESS_SEND_KEY_LVL2:
            // V rifier la longueur des donn es envoy es
            if (data_length != 2) {
                send_negative_response_security_access(sub_function, NRC_INCORRECT_MESSAGE_LENGTH);
                return;
            }
            // V rifier la cl  envoy e
            if (verify_key(UDS_SECURITY_ACCESS_SEND_KEY_LVL2, data)) {
                send_positive_response_security_access(UDS_SECURITY_ACCESS_SEND_KEY_LVL2);
            } else {
                send_negative_response_security_access(sub_function, NRC_INVALID_KEY);
            }
            break;

        default:
            // Sous-fonction non support e
            send_negative_response_security_access(sub_function, NRC_SUB_FUNCTION_NOT_SUPPORTED);
            break;
    }
}


/**
 * @brief Envoie le seed pour le niveau de s curit  donn
 * @param level : Niveau de s curit  (ex: 0x01 pour niveau 1, 0x03 pour niveau 2)
 */
void send_seed(uint8_t level) {
    uint8_t response[5] = {0};
    response[0] = 0x04;
    response[1] = UDS_SECURITY_ACCESS + 0x40;  // R ponse positive
    response[2] = level;  // Niveau de s curit  demand

    // Seed sp cifique au niveau de s curit
    if (level == UDS_SECURITY_ACCESS_REQUEST_SEED_LVL1) {
        response[3] = security_seed_lvl1[0];
        response[4] = security_seed_lvl1[1];
    } else if (level == UDS_SECURITY_ACCESS_REQUEST_SEED_LVL2) {
        response[3] = security_seed_lvl2[0];
        response[4] = security_seed_lvl2[1];
    }

    send_can_message(response, 5);  // Envoyer le seed via CAN
}

/**
 * @brief V rifie la cl  envoy e par le client
 * @param level : Niveau de s curit  (ex: 0x02 pour niveau 1, 0x04 pour niveau 2)
 * @param key : Cl  envoy e par le client
 */
bool verify_key(uint8_t level, uint8_t* key) {
    // V rification simple de la cl
    if (key[0] == security_key_lvl1[0] || key[1] == security_key_lvl1[1]) {
        uds_session.security_access_granted = true;  // Accorder l'acc s de s curit
        return true;
    } else if (key[0] == security_key_lvl2[0] || key[1] == security_key_lvl2[1]) {
        uds_session.security_access_granted = true;  // Accorder l'acc s de s curit
        return true;
    } else {
        return false;
    }
}



/**
 * @brief Envoie une r ponse positive pour le service Security Access
 * @param sub_function : Sous-fonction (0x01 pour demande de seed, 0x02 pour v rification de cl )
 */
void send_positive_response_security_access(uint8_t sub_function) {
    uint8_t response[5] = {0};
     	 	response[0] =0x02;

    // R ponse SID pour indiquer un succ s (ajout de 0x40 au service ID)
    response[1] = UDS_SECURITY_ACCESS + 0x40;  // R ponse positive (0x27 + 0x40 = 0x67)
    response[2] = sub_function;  // Type d'acc s s curis  ( cho de la sous-fonction)

    // Si c'est une demande de seed, on inclut la graine dans la r ponse
    /* if (sub_function == UDS_SECURITY_ACCESS_REQUEST_SEED_LVL1) {
        response[2] = security_seed_lvl1;  // Graine de s curit  niveau 1
    } else if (sub_function == UDS_SECURITY_ACCESS_REQUEST_SEED_LVL2) {
        response[2] = security_seed_lvl2;  // Graine de s curit  niveau 2
    }*/

    // Longueur du message : 2 octets pour la sous-fonction et la graine (si pr sente)
    uint8_t response_length = (sub_function == UDS_SECURITY_ACCESS_REQUEST_SEED_LVL1 ||
                               sub_function == UDS_SECURITY_ACCESS_REQUEST_SEED_LVL2) ? 4 : 3;

    // Envoyer le message CAN avec la r ponse positive
    send_can_message(response, response_length);
}

/**
 * @brief Envoie une r ponse n gative pour le service Security Access
 * @param sub_function : Sous-fonction concern e
 * @param nrc : Code de r ponse n gative (NRC)
 */
void send_negative_response_security_access(uint8_t sub_function, uint8_t nrc) {
    uint8_t response[3] = {0};

    response[0] = UDS_NEGATIVE_RESPONSE;         // Indique une r ponse n gative
    response[1] = UDS_SECURITY_ACCESS;  // Identifiant du service
    response[2] = nrc;          // NRC (code de r ponse n gative)

    send_can_message(response, 3);  // Envoyer la r ponse via CAN
}


/************************************************CommunicationControl************************************************************/
// Impl�mentation du service Communication Control (0x28)
void uds_communication_control(uint8_t sub_function) {
    switch (sub_function) {
        case UDS_COMM_CONTROL_ENABLE_RX_AND_TX:
            // Activer Rx et Tx
            if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
                send_negative_response_communication_control(NRC_CONDITIONS_NOT_CORRECT);
                return;
            }
            send_positive_response_communication_control(sub_function);
            break;

        case UDS_COMM_CONTROL_ENABLE_RX_AND_DISABLE_TX:
            // Activer uniquement Rx, d�sactiver Tx
            if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
                send_negative_response_communication_control(NRC_CONDITIONS_NOT_CORRECT);
                return;
            }
            if (HAL_CAN_DeactivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
                send_negative_response_communication_control(NRC_CONDITIONS_NOT_CORRECT);
                return;
            }
            send_positive_response_communication_control(sub_function);
            break;

        case UDS_COMM_CONTROL_DISABLE_RX_AND_ENABLE_TX:
            // D�sactiver Rx, activer Tx
            if (HAL_CAN_DeactivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
                send_negative_response_communication_control(NRC_CONDITIONS_NOT_CORRECT);
                return;
            }
            if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
                send_negative_response_communication_control(NRC_CONDITIONS_NOT_CORRECT);
                return;
            }
            send_positive_response_communication_control(sub_function);
            break;

        case UDS_COMM_CONTROL_DISABLE_RX_AND_TX:
            // D�sactiver � la fois Rx et Tx
            if (HAL_CAN_DeactivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
                send_negative_response_communication_control(NRC_CONDITIONS_NOT_CORRECT);
                return;
            }
            send_positive_response_communication_control(sub_function);
            break;

        case UDS_COMM_CONTROL_ENABLE_RX_AND_TX_WITH_ENHANCED_INFO:
            // Activer Rx et Tx avec informations am�lior�es
            if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
                send_negative_response_communication_control(NRC_CONDITIONS_NOT_CORRECT);
                return;
            }
            // Ajoutez la logique pour les informations am�lior�es ici
            send_positive_response_communication_control(sub_function);
            break;

        case UDS_COMM_CONTROL_ENABLE_RX_WITH_ENHANCED_INFO:
            // Activer Rx avec informations am�lior�es
            if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
                send_negative_response_communication_control(NRC_CONDITIONS_NOT_CORRECT);
                return;
            }
            // Ajoutez la logique pour les informations am�lior�es ici
            send_positive_response_communication_control(sub_function);
            break;

        default:
            // Sous-fonction non support�e
            send_negative_response_communication_control(NRC_SUB_FUNCTION_NOT_SUPPORTED);
            break;
    }
}


// Envoie une r�ponse positive pour Communication Control
void send_positive_response_communication_control(uint8_t sub_function) {
    // Tableau de r�ponse contenant 2 octets
    uint8_t response[2];

    // Octet #1 : SID de r�ponse (0x68 = 0x28 + 0x40)
    response[0] = UDS_COMMUNICATION_CONTROL + 0x40;

    // Octet #2 : Echo de la sous-fonction envoy�e par le client
    response[1] = sub_function;

    // Envoi du message CAN
    send_can_message(response, 2);
}

// Envoie une r�ponse n�gative pour Communication Control
void send_negative_response_communication_control(uint8_t nrc) {
    // Tableau de r�ponse contenant 3 octets
    uint8_t response[3];

    // Octet #1 : 0x7F indiquant une r�ponse n�gative
    response[0] = UDS_NEGATIVE_RESPONSE;

    // Octet #2 : Service ID (SID) pour Communication Control (0x28)
    response[1] = UDS_COMMUNICATION_CONTROL;

    // Octet #3 : Code de r�ponse n�gative (NRC)
    response[2] = nrc;

    // Envoi du message CAN
    send_can_message(response, 3);
}

/*****************************************************TesterPresent**************************************************************/
void uds_tester_present(uint8_t sub_function, uint8_t len) {
    // Utilisation d'une macro pour v�rifier la sous-fonction r�serv�e
    if ((sub_function & UDS_TESTER_PRESENT_SUB_FUNCTION_MASK) != UDS_TESTER_PRESENT_ZERO_SUB_FUNCTION) {
        // Si la sous-fonction est diff�rente de 0x00, elle est non support�e
        send_negative_response_tester_present(NRC_SUB_FUNCTION_NOT_SUPPORTED);
        return;
    }

    // V�rification du suppressPosRspMsgIndicationBit avec une macro
    if ((sub_function & UDS_SUPPRESS_POS_RSP_MSG_INDICATION_BIT) == UDS_SUPPRESS_POS_RSP_MSG_INDICATION_BIT) {
        // Si suppressPosRspMsgIndicationBit est � 1, aucune r�ponse ne doit �tre envoy�e
        return;
    }

    //reseting the s3 timer to 0 to keep the session in current session
    void reset_s3_timer()
    {
        uds_session.s3_timer = 0;
        uds_session.s3_timeout_active = true;
    }

    // Si suppressPosRspMsgIndicationBit est � 0, envoyer une r�ponse positive
    send_positive_response_tester_present(sub_function);
}

void send_positive_response_tester_present(uint8_t sub_function) {
    // Tableau de r�ponse contenant 2 octets
    uint8_t response[3];

    // Octet #1 : SID de r�ponse pour TesterPresent (0x3E + 0x40 = 0x7E)
    response[0] = 2;
    response[1] = UDS_TESTER_PRESENT + 0x40;

    // Octet #2 : Echo de la sous-fonction (bits 6 � 0) envoy�e par le client
    response[2] = sub_function & UDS_TESTER_PRESENT_SUB_FUNCTION_MASK;  // On ne prend que les bits 6 � 0

    // Envoi du message CAN
    send_can_message(response, 3);
}

void send_negative_response_tester_present(uint8_t nrc) {
    // Tableau de r�ponse contenant 3 octets
    uint8_t response[4];

    // Octet #1 : 0x7F indiquant une r�ponse n�gative
    response[0] = 4;
    response[1] = UDS_NEGATIVE_RESPONSE;

    // Octet #2 : Service ID (SID) pour Tester Present (0x3E)
    response[2] = UDS_TESTER_PRESENT;

    // Octet #3 : Code de r�ponse n�gative (NRC)
    response[3] = nrc;

    // Envoi du message CAN
    send_can_message(response, sizeof(response));
}


/*********************************************************access_timing_parameter**********************************************************/
// Exemple de param�tres de temporisation par d�faut
TimingParameters timing_params = {
    .p2_server_max = 0x32,       // 50 ms
    .p2_star_server_max = 0x1F4  // 500 ms
};

// Fonction principale pour Access Timing Parameter
void uds_access_timing_parameter(uint8_t sub_function, uint8_t* data, uint8_t data_length) {
    if (!is_service_allowed(UDS_ACCESS_TIMING_PARAMETER)) {
        send_negative_response_access_timing(NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    switch (sub_function) {
        case UDS_TIMING_PARAMETER_READ:
            // Lire les param�tres de temporisation
            if (data_length != 0) {
                send_negative_response_access_timing(NRC_INCORRECT_MESSAGE_LENGTH);
                return;
            }
            read_timing_parameters();
            break;

        case UDS_TIMING_PARAMETER_WRITE:
            // �crire de nouveaux param�tres de temporisation
            if (data_length != 4) {
                send_negative_response_access_timing(NRC_INCORRECT_MESSAGE_LENGTH);
                return;
            }
            write_timing_parameters(data, data_length);
            break;

        default:
            send_negative_response_access_timing(NRC_SUB_FUNCTION_NOT_SUPPORTED);
            break;
    }
}

// Fonction pour lire les param�tres de temporisation
void read_timing_parameters() {
    uint8_t response[6];

    // SID de r�ponse (0x83 + 0x40 = 0xC3)
    response[0] = UDS_ACCESS_TIMING_PARAMETER + 0x40;

    // P2Server_max
    response[1] = (timing_params.p2_server_max >> 8) & 0xFF; // High byte
    response[2] = timing_params.p2_server_max & 0xFF;        // Low byte

    // P2*Server_max
    response[3] = (timing_params.p2_star_server_max >> 8) & 0xFF; // High byte
    response[4] = timing_params.p2_star_server_max & 0xFF;        // Low byte

    // Envoyer la r�ponse CAN
    send_can_message(response, 5);
}

// Fonction pour �crire de nouveaux param�tres de temporisation
void write_timing_parameters(uint8_t* data, uint8_t data_length) {
    // Mise � jour des param�tres de temporisation avec les donn�es re�ues
    timing_params.p2_server_max = (data[0] << 8) | data[1];        // P2Server_max
    timing_params.p2_star_server_max = (data[2] << 8) | data[3];   // P2*Server_max

    // Envoyer une r�ponse positive
    send_positive_response_access_timing(UDS_TIMING_PARAMETER_WRITE);
}

void send_positive_response_access_timing(uint8_t sub_function) {
    uint8_t response[2];

    // SID de r�ponse pour AccessTimingParameter (0x83 + 0x40 = 0xC3)
    response[0] = UDS_ACCESS_TIMING_PARAMETER + 0x40;

    // �cho de la sous-fonction (bits 6 � 0)
    response[1] = sub_function & 0x7F;

    // Envoyer la r�ponse CAN
    send_can_message(response, 2);
}


void send_negative_response_access_timing(uint8_t nrc) {
    uint8_t response[3];

    // Octet #1 : 0x7F indiquant une r�ponse n�gative
    response[0] = UDS_NEGATIVE_RESPONSE;

    // Octet #2 : Service ID (SID) pour Access Timing Parameter (0x83)
    response[1] = UDS_ACCESS_TIMING_PARAMETER;

    // Octet #3 : Code de r�ponse n�gative (NRC)
    response[2] = nrc;

    // Envoyer le message CAN
    send_can_message(response, 3);
}

/***********************************************SecuredDataTransmission**********************************************************/
/**
 * @brief Impl�mente le service de transmission de donn�es s�curis�es (0x84)
 * @param data : Donn�es � transmettre
 * @param data_length : Longueur des donn�es
 */
SecuritySession current_session = {
    .session_id = UDS_SESSION_DEFAULT,
    .encryption_key = DEFAULT_ENCRYPTION_KEY
};

void uds_secured_data_transmission(uint8_t* data, uint8_t data_length) {
    // V�rification si le service est autoris� dans la session actuelle
    if (!is_service_allowed(UDS_SECURED_DATA_TRANSMISSION)) {
        send_negative_response_secured_data_transmission(NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // V�rification de la longueur du message
    if (data_length > MAX_DATA_SIZE) {
        send_negative_response_secured_data_transmission(NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Chiffrement des donn�es
    uint8_t encrypted_data[MAX_DATA_SIZE];
    encrypt_data(data, data_length, encrypted_data, current_session.encryption_key);

    // Envoi des donn�es chiffr�es
    secured_data_send(encrypted_data, data_length);

    // Donn�es de r�ponse (� adapter selon vos besoins)
    uint8_t securityDataResponseRecord[MAX_DATA_SIZE];
    // Remplir le tableau securityDataResponseRecord avec des donn�es appropri�es

    // R�ponse positive apr�s transmission
    send_positive_response_secured_data_transmission(securityDataResponseRecord, data_length);
}


/**
 * @brief Chiffre les donn�es � l'aide de la cl� sp�cifi�e
 * @param data : Donn�es � chiffrer
 * @param length : Longueur des donn�es
 * @param encrypted_data : Tableau o� stocker les donn�es chiffr�es
 * @param key : Cl� de chiffrement
 */
void encrypt_data(uint8_t* data, uint8_t length, uint8_t* encrypted_data, uint8_t* key) {
    // Exemple basique de chiffrement XOR (vous devez remplacer par un chiffrement plus robuste comme AES)
    for (uint8_t i = 0; i < length; i++) {
        encrypted_data[i] = data[i] ^ key[i % 16];
    }
}

/**
 * @brief D�chiffre les donn�es � l'aide de la cl� sp�cifi�e
 * @param encrypted_data : Donn�es chiffr�es � d�chiffrer
 * @param length : Longueur des donn�es chiffr�es
 * @param decrypted_data : Tableau o� stocker les donn�es d�chiffr�es
 * @param key : Cl� de d�chiffrement
 */
void decrypt_data(uint8_t* encrypted_data, uint8_t length, uint8_t* decrypted_data, uint8_t* key) {
    // Exemple basique de d�chiffrement XOR (vous devez remplacer par un chiffrement plus robuste comme AES)
    for (uint8_t i = 0; i < length; i++) {
        decrypted_data[i] = encrypted_data[i] ^ key[i % 16];
    }
}

/**
 * @brief Envoie les donn�es chiffr�es via le bus CAN
 * @param encrypted_data : Donn�es chiffr�es � envoyer
 * @param length : Longueur des donn�es chiffr�es
 */
void secured_data_send(uint8_t* encrypted_data, uint8_t length) {
    // Vous pouvez ajuster les en-t�tes CAN ici si n�cessaire
    send_can_message(encrypted_data, length);
}

void send_positive_response_secured_data_transmission(uint8_t* securityDataResponseRecord, uint8_t data_length) {
    uint8_t response[MAX_DATA_SIZE + 1]; // +1 pour l'octet de SID

    // Byte 1: Secured Data Transmission Response SID (0xC4)
    response[0] = UDS_SECURED_DATA_TRANSMISSION + 0x40; // 0xC4

    // Bytes 2 to n: securityDataResponseRecord[]
    for (uint8_t i = 0; i < data_length; i++) {
        response[i + 1] = securityDataResponseRecord[i]; // Ajouter les param�tres de r�ponse
    }

    // Envoyer la r�ponse via CAN
    send_can_message(response, data_length + 1); // La longueur totale inclut SID + securityDataResponseRecord
}

void send_negative_response_secured_data_transmission(uint8_t nrc) {
    uint8_t response[3];

    // Byte 1: Negative Response SID (0x7F)
    response[0] = UDS_NEGATIVE_RESPONSE;

    // Byte 2: Secured Data Transmission Service ID (0x84)
    response[1] = UDS_SECURED_DATA_TRANSMISSION;

    // Byte 3: NRC (Code de r�ponse n�gative)
    response[2] = nrc;

    // Envoyer la r�ponse via CAN
    send_can_message(response, 3);
}
/***********************************************ControlDTCSetting****************************************************************/
// Drapeaux pour g�rer l'�tat de mise � jour des bits DTC
bool dtc_update_enabled = true; // Par d�faut, les bits DTC sont mis � jour

/**
 * @brief Service ControlDTCSetting (0x85)
 * @param sub_function : Sous-fonction (0x01 pour activer, 0x02 pour d�sactiver)
 */
void uds_control_dtc_setting(uint8_t sub_function) {
    // V�rification si le service est autoris� dans la session actuelle
    if (!is_service_allowed(UDS_CONTROL_DTC_SETTING)) {
        send_negative_response_control_dtc_setting(NRC_CONDITIONS_NOT_CORRECT);
        return;
    }

    // V�rifier si la sous-fonction est valide (0x01 ou 0x02)
    if (sub_function != UDS_CONTROL_DTC_SETTING_ON && sub_function != UDS_CONTROL_DTC_SETTING_OFF) {
        send_negative_response_control_dtc_setting(NRC_SUB_FUNCTION_NOT_SUPPORTED);
        return;
    }

    // V�rifier et traiter la sous-fonction
    if (sub_function == UDS_CONTROL_DTC_SETTING_ON) {
        if (!dtc_update_enabled) {
            dtc_update_enabled = true; // Activer la mise � jour des DTC
        }
        // Envoyer une r�ponse positive m�me si d�j� activ�
        send_positive_response_control_dtc_setting(sub_function);

    } else if (sub_function == UDS_CONTROL_DTC_SETTING_OFF) {
        if (dtc_update_enabled) {
            dtc_update_enabled = false; // D�sactiver la mise � jour des DTC
        }
        // Envoyer une r�ponse positive m�me si d�j� d�sactiv�
        send_positive_response_control_dtc_setting(sub_function);
    }
}

/**
 * @brief Envoie une r�ponse positive pour le service ControlDTCSetting
 * @param sub_function : Sous-fonction (0x01 pour activer, 0x02 pour d�sactiver)
 */
void send_positive_response_control_dtc_setting(uint8_t sub_function) {
    uint8_t response[2] = {0};

    // R�ponse SID pour indiquer un succ�s (ajout de 0x40 au service ID)
    response[0] = UDS_CONTROL_DTC_SETTING + 0x40; // 0xC5

    // Sous-fonction : Echo de la sous-fonction envoy�e (0x01 ou 0x02)
    response[1] = sub_function;

    // Envoi de la r�ponse via CAN
    send_can_message(response, 2);
}

/**
 * @brief Envoie une r�ponse n�gative pour le service ControlDTCSetting
 * @param nrc : Code de r�ponse n�gative (NRC)
 */
void send_negative_response_control_dtc_setting(uint8_t nrc) {
    uint8_t response[3] = {0};

    // R�ponse n�gative g�n�rique (0x7F)
    response[0] = UDS_NEGATIVE_RESPONSE;

    // Service concern� (0x85 pour ControlDTCSetting)
    response[1] = UDS_CONTROL_DTC_SETTING;

    // NRC (Code de r�ponse n�gative)
    response[2] = nrc;

    // Envoi de la r�ponse via CAN
    send_can_message(response, 3);
}

/*************************************************ResponseOnEvent****************************************************************/
Event events[MAX_EVENTS]; // D�finition de la variable events

void uds_response_on_event(uint8_t sub_function, uint8_t* data, uint8_t data_length) {
    switch (sub_function) {
        case ROE_STOP_RESPONSE_ON_EVENT:
            // Appel de la fonction pour arr�ter la r�ponse sur �v�nement
            stop_response_on_event();
            break;

        case ROE_ON_DTC_STATUS_CHANGE:
            // Gestion de l'�v�nement bas� sur le changement de statut DTC
            on_dtc_status_change(data, data_length);
            break;

        case ROE_ON_TIMER_INTERRUPT:
            // Gestion de l'�v�nement bas� sur une interruption de timer
            on_timer_interrupt(data, data_length);
            break;

        case ROE_START_RESPONSE_ON_EVENT:
            // D�marrage de la r�ponse sur �v�nement
            start_response_on_event();
            break;

        case ROE_CLEAR_RESPONSE_ON_EVENT:
            // Nettoyage des �v�nements configur�s
            clear_response_on_event();
            break;

        default:
            // Envoi d'une r�ponse n�gative si la sous-fonction n'est pas support�e
            send_negative_response_roe(sub_function, NRC_SUB_FUNCTION_NOT_SUPPORTED);
            break;
    }
}

void stop_response_on_event() {
    // D�sactiver tous les �v�nements actifs
    for (int i = 0; i < MAX_EVENTS; i++) {
        events[i].isActive = false;
    }

    // Envoyer une r�ponse positive avec des valeurs par d�faut
    send_positive_response_roe(ROE_STOP_RESPONSE_ON_EVENT, 0x00, 0x00, 0x00);
}

void on_dtc_status_change(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur des donn�es
    if (data_length != 1) {
        send_negative_response_roe(ROE_ON_DTC_STATUS_CHANGE, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Configurer un nouvel �v�nement pour DTC Status Change
    for (int i = 0; i < MAX_EVENTS; i++) {
        if (!events[i].isActive) {
            events[i].eventType = ROE_ON_DTC_STATUS_CHANGE;
            events[i].isActive = true;
            events[i].serviceToRespondTo = UDS_READ_DTC_INFORMATION;
            break;
        }
    }

    // Envoyer une r�ponse positive
    send_positive_response_roe(ROE_ON_DTC_STATUS_CHANGE, 0x01, 0x01, 0x00);
}

void on_timer_interrupt(uint8_t* data, uint8_t data_length) {
    // V�rifier la longueur des donn�es
    if (data_length != 1) {
        send_negative_response_roe(ROE_ON_TIMER_INTERRUPT, NRC_INCORRECT_MESSAGE_LENGTH);
        return;
    }

    // Configurer un �v�nement pour Timer Interrupt
    for (int i = 0; i < MAX_EVENTS; i++) {
        if (!events[i].isActive) {
            events[i].eventType = ROE_ON_TIMER_INTERRUPT;
            events[i].isActive = true;
            events[i].serviceToRespondTo = UDS_READ_DATA_BY_IDENTIFIER;
            break;
        }
    }

    // Envoyer une r�ponse positive
    send_positive_response_roe(ROE_ON_TIMER_INTERRUPT, 0x01, 0x01, 0x00);
}


void start_response_on_event() {
    // Activer les �v�nements configur�s
    for (int i = 0; i < MAX_EVENTS; i++) {
        if (events[i].isActive) {
            // Lancer la logique de gestion des �v�nements
        }
    }

    // Envoyer une r�ponse positive
    send_positive_response_roe(ROE_START_RESPONSE_ON_EVENT, 0x00, 0x00, 0x00);
}

void clear_response_on_event() {
    // R�initialiser tous les �v�nements
    for (int i = 0; i < MAX_EVENTS; i++) {
        events[i].isActive = false;
        events[i].eventType = 0;
        events[i].serviceToRespondTo = 0;
    }

    // Envoyer une r�ponse positive
    send_positive_response_roe(ROE_CLEAR_RESPONSE_ON_EVENT, 0x00, 0x00, 0x00);
}

void send_positive_response_roe(uint8_t sub_function, uint8_t eventType, uint8_t numberOfIdentifiedEvents, uint8_t eventWindowTime) {
    // Initialiser une taille suffisante pour inclure tous les param�tres (3 octets de base + les autres donn�es)
    uint8_t response[6] = {0};

    // Byte 1: ResponseOnEvent Response SID (0x86 + 0x40 = 0xC6)
    response[0] = UDS_RESPONSE_ON_EVENT + 0x40;  // 0xC6

    // Byte 2: eventType (echo de la sous-fonction dans la requ�te)
    response[1] = eventType;

    // Byte 3: numberOfIdentifiedEvents (0x00 si non applicable ou nombre r�el d'�v�nements)
    response[2] = numberOfIdentifiedEvents;

    // Byte 4: eventWindowTime (temps de la fen�tre d'�v�nement)
    response[3] = eventWindowTime;

    // Envoi du message CAN avec la r�ponse positive (4 octets dans ce cas)
    send_can_message(response, 4);
}

void send_negative_response_roe(uint8_t sub_function, uint8_t nrc) {
    uint8_t response[3] = {0};

    // Byte 1: NRC indicator (0x7F for negative response)
    response[0] = UDS_NEGATIVE_RESPONSE;

    // Byte 2: Service ID (SID of ResponseOnEvent, which is 0x86)
    response[1] = UDS_RESPONSE_ON_EVENT;

    // Byte 3: Negative Response Code (NRC) from Table 108
    response[2] = nrc;

    // Send the negative response via CAN
    send_can_message(response, 3);
}

/****************************************************LinkControl****************************************************************/
LinkControl link_control;

void uds_link_control(uint8_t sub_function, uint8_t* data, uint8_t data_length) {
    switch (sub_function) {
        case UDS_LINK_CONTROL_VERIFY_MODE_TRANSITION_FIXED:
            if (data_length != 1) {
                send_negative_response_link_control(sub_function, NRC_INCORRECT_MESSAGE_LENGTH);
                return;
            }
            link_control.modeIdentifier = data[0];
            send_positive_response_link_control(sub_function);
            break;

        case UDS_LINK_CONTROL_VERIFY_MODE_TRANSITION_SPECIFIC:
            if (data_length != 3) {
                send_negative_response_link_control(sub_function, NRC_INCORRECT_MESSAGE_LENGTH);
                return;
            }
            memcpy(link_control.linkRecord, data, 3);
            send_positive_response_link_control(sub_function);
            break;

        case UDS_LINK_CONTROL_TRANSITION_MODE:
            if (link_control.modeIdentifier == 0) {
                send_negative_response_link_control(sub_function, NRC_REQUEST_SEQUENCE_ERROR);
                return;
            }
            // Effectuer la transition ici en fonction de modeIdentifier ou linkRecord
            send_positive_response_link_control(sub_function);
            break;

        default:
            send_negative_response_link_control(sub_function, NRC_SUB_FUNCTION_NOT_SUPPORTED);
            break;
    }
}

void send_positive_response_link_control(uint8_t sub_function) {
    uint8_t response[2] = {0};

    response[0] = UDS_LINK_CONTROL + 0x40;  // R�ponse SID
    response[1] = sub_function;  // Echo de la sous-fonction

    send_can_message(response, 2);
}

void send_negative_response_link_control(uint8_t sub_function, uint8_t nrc) {
    uint8_t response[3] = {0};

    response[0] = UDS_NEGATIVE_RESPONSE; // R�ponse n�gative
    response[1] = UDS_LINK_CONTROL; // Service concern� (LinkControl)
    response[2] = nrc; // NRC (Code de r�ponse n�gative)

    send_can_message(response, 3);
}




