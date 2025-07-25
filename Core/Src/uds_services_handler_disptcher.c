/*
 * uds_services_handler_disptcher.c
 *
 *  Created on: Jul 23, 2025
 *      Author: naveenkumar.m
 */


#include "main.h"
#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "uds_services.h"
#include "uds_services_handler_disptcher.h"


void uds_services_handler_disptcher(uint8_t * data, uint16_t len)
{
	uint8_t response[3];
	// Identify the UDS service based on the first byte of the message
	switch (data[0]) {
	case UDS_DIAGNOSTIC_SESSION_CONTROL:
		//checking suppressPosRspMsgIndicationBit true or false
		suppress = data[2] & 0x80;
		data[2] = data[2] & 0x0F;
		// Call the function for the Diagnostic Session Control service
		uds_diagnostic_session_control(data[1], len);
		break;
	case UDS_ECU_RESET:
		//checking suppressPosRspMsgIndicationBit true or false
		suppress = data[2] & 0x80;
		data[2] = data[2] & 0x0F;
		// Call the function for the ECU Reset service with the resetType
		uds_ecu_reset(data[2], len);
		break;
	case UDS_SECURITY_ACCESS:
		// Call the function directly for the Security Access service
		uds_security_access(data[1], &data[2], len);
		break;
	case UDS_COMMUNICATION_CONTROL:
		uds_communication_control(data[1]);
		break;
	case UDS_TESTER_PRESENT:
		// Call the function for the TesterPresent service
		uds_tester_present(data[1], len);
		break;
	case UDS_ACCESS_TIMING_PARAMETER:
		// Call the function for the Access Timing Parameter service
		uds_access_timing_parameter(data[1], &data[2], len - 2);
		break;
	case UDS_SECURED_DATA_TRANSMISSION:
		// Call the function for the Secured Data Transmission service
		uds_secured_data_transmission(&data[1], len - 1);
		break;
	case UDS_CONTROL_DTC_SETTING:
		// Call the function for the ControlDTCSetting service
		uds_control_dtc_setting(data[1]);
		break;
	case UDS_RESPONSE_ON_EVENT:
		// Call the function for the ResponseOnEvent service
		uds_response_on_event(data[1], &data[2], len - 2);
		break;
	case UDS_LINK_CONTROL:
		// Call the function for the LinkControl service
		uds_link_control(data[1], &data[2], len - 2);
		break;
	case UDS_READ_DATA_BY_IDENTIFIER:
		// Call the function for the ReadDataByIdentifier service
		uds_read_data_by_identifier(&data[1], len - 1);
		break;
	case UDS_READ_DATA_BY_PERIODIC_IDENTIFIER:
		// Call the function for the ReadDataByIdentifier service
		uds_read_data_by_periodic_identifier(&data[1], len - 1);
		break;
		// Call the function for the Dynamically Define Data Identifier service
		uds_dynamically_define_data_identifier(data[1], &data[2], len - 2);
		break;
	case UDS_WRITE_DATA_BY_IDENTIFIER:
		// Call the function for the WriteDataByIdentifier service
		uds_write_data_by_identifier(&data[1],len - 1);
		break;
	case UDS_CLEAR_DIAGNOSTIC_INFORMATION:
		// Call the function for the ClearDiagnosticInformation service
		uds_clear_diagnostic_information(&data[1], len - 1);
		break;
	case UDS_READ_DTC_INFORMATION:
		// Call the function to manage the ReadDTCInformation service
		uds_read_dtc_information(data[1], &data[2], len - 2);
		break;
	case UDS_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER:
		uds_input_output_control_by_identifier((IOControlRequest_t *)&data[1], NULL);
		break;
	case UDS_ROUTINE_CONTROL:
		uds_routine_control((RoutineControlRequest_t *)&data[1], NULL);
		break;
	case UDS_REQUEST_DOWNLOAD:
		uds_request_download((RequestDownload_t *)&data[1]);
		break;
	case UDS_REQUEST_UPLOAD:
		uds_request_upload((RequestUpload_t *)&data[1]);
		break;
	case UDS_TRANSFER_DATA:
		uds_transfer_data((RequestTransferData_t *)&data[1]);
		break;
	case UDS_REQUEST_TRANSFER_EXIT:
		uds_request_transfer_exit((RequestTransferExit_t *)&data[1], NULL);
		break;
	case UDS_REQUEST_FILE_TRANSFER:
		uds_request_file_transfer((RequestFileTransfer_t *)&data[1]);
		break;
	default:
		// Complete the negative response message for an unsupported service.
		response[0] = UDS_NEGATIVE_RESPONSE;         // Response negative require
		response[1] = data[0];  // Service non support
		response[2] = NRC_SERVICE_NOT_SUPPORTED;         // Code NRC (ServiceNotSupported)
		// Send the negative response message with CAN
		send_can_message(response, 3);
		break;
	}
}


