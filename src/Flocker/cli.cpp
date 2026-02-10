/********************************************************************
 * cli.cpp
 ********************************************************************
 * Entry point to command line handler.  The actual CLI processing
 * is handled by embedded_cli.h.  Command definitions are in
 * cli_impl.h
 *******************************************************************/

#include <Arduino.h>
#include <ctime>

#include "cli.h"
#include "flockCfg.h"
#include "flockLog.h"

#include "globals.h"

// This define has to be included in exactly one translation unit
// of the project.  See ./embedded_cli.h for info
#define EMBEDDED_CLI_IMPL
#include "embedded_cli.h"
#include "cli_impl.h"

static EmbeddedCli *cli = NULL;
static CLI_UINT* cliBuffer = NULL;  // will be allocated in PSRAM by embeddedCliNew()
static bool cliIsHeld = false;
static bool firstCharSeen = false;

bool setupCLI()
{
  EmbeddedCliConfig *config = embeddedCliDefaultConfig();

  config->cliBuffer = cliBuffer;

  config->cliBufferSize = CLI_BUFFER_SIZE_PS;
  config->rxBufferSize = CLI_RX_BUFFER_SIZE_PS;
  config->cmdBufferSize = CLI_CMD_BUFFER_SIZE_PS;
  config->historyBufferSize = CLI_HISTORY_SIZE_PS;
  config->maxBindingCount = CLI_BINDING_COUNT_PS;
  config->invitation = CLI_BOLD_PUR "Flock Off" CLI_RESET " $>";
  cli = embeddedCliNew(config, true);

  if (!cli)
  {
      Serial.printf("Cli was not created. Check sizes!\r\n");
      return (false);
  }

  // add command bindings
  for (size_t ii = 0; ii < bindingCount; ++ii)
  {
    embeddedCliAddBinding(cli, bindings[ii]);
  }

  cli->onCommand = onCommand; // handler for unknown command
  cli->writeChar = writeChar; // copy from serial in to cli handler

  //if (initOk)
  //{
  //  Serial.printf(CLI_CLEAR);
  //}

  Serial.printf("\r\n\r\n\r\n");

  Serial.printf(CLI_BOLD_RED "   /$$$$$$$$ /$$                     /$$              /$$$$$$   /$$$$$$   /$$$$$$\r\n");
  Serial.printf(CLI_BOLD_GRN "  | $$_____/| $$                    | $$             /$$__  $$ /$$__  $$ /$$__  $$\r\n");
  Serial.printf(CLI_BOLD_YEL "  | $$      | $$  /$$$$$$   /$$$$$$$| $$   /$$      | $$  \\ $$| $$  \\__/| $$  \\__/\r\n");
  Serial.printf(CLI_BOLD_BLU "  | $$$$$   | $$ /$$__  $$ /$$_____/| $$  /$$/      | $$  | $$| $$$$    | $$$$    \r\n");
  Serial.printf(CLI_BOLD_PUR "  | $$__/   | $$| $$  \\ $$| $$      | $$$$$$/       | $$  | $$| $$_/    | $$_/    \r\n");
  Serial.printf(CLI_BOLD_CYA "  | $$      | $$| $$  | $$| $$      | $$_  $$       | $$  | $$| $$      | $$      \r\n");
  Serial.printf(CLI_BOLD_RED "  | $$      | $$|  $$$$$$/|  $$$$$$$| $$ \\  $$      |  $$$$$$/| $$      | $$      \r\n");
  Serial.printf(CLI_BOLD_GRN "  |__/      |__/ \\______/  \\_______/|__/  \\__/       \\______/ |__/      |__/      \r\n");

  Serial.printf(CLI_RESET CLI_BOLD "(c) 2025-2026 M.Brugman\r\n\r\n" CLI_RESET);
  Serial.printf("CLI Enabled.  'help' for help; <command -h> for help on that command\r\n");

  return (true);
}

void updateCLI()
{
  if (!cliIsHeld)
  {
    while (Serial.available())
    {
        firstCharSeen = true;
        embeddedCliReceiveChar(cli, Serial.read());
    }

    embeddedCliProcess(cli);
  }
}

void holdCLI(bool hold)
{
  cliIsHeld = hold;
}

bool cliActive()
{
    return (firstCharSeen);
}
