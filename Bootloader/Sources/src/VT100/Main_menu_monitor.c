#include "App.h"


static void     Do_Reset(uint8_t keycode);
static void     Do_date_time_set(uint8_t keycode);
static void     Do_show_event_log(uint8_t keycode);
static void     Do_MOD_Params_editor(uint8_t keycode);
static void     Do_Copy_Module_Settings_to_File(uint8_t keycode);
static void     Do_return_defaul_bootloader_settings(uint8_t keycode);
static void     Do_erase_application_firmware(uint8_t keycode);
static void     Do_erase_application_NV_settings(uint8_t keycode);

extern const T_VT100_Menu MENU_MAIN;
extern const T_VT100_Menu MENU_PARAMETERS;
extern const T_VT100_Menu MENU_ENGINEERING;


static int32_t Lookup_menu_item(T_VT100_Menu_item **item, uint8_t b);


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

      Пункты имеющие свое подменю располагаются на следующем уровне вложенности
      Их функция подменяет в главном цикле обработчик нажатий по умолчанию и должна
      отдавать управление периодически в главный цикл

      Пункты не имеющие функции просто переходят на следующий уровень подменю

      Пункты не имеющие подменю полностью захватывают управление и на следующий уровень не переходят

      Пункты без подменю и функции означают возврат на предыдущий уровень
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

const T_VT100_Menu      MENU_PARAMETERS       =
{
  "",
  "",
  0
};

const T_VT100_Menu_item MENU_ENGINEERING_ITEMS[] =
{
  { '1', Do_system_info,                       0                },
  { '2', Do_Bluetooth_control,                 0                },
  { '3', Do_Flash_Protection_control,          0                },
  { '4', Do_SD_card_control,                   0                },
  { '5', Do_return_defaul_bootloader_settings, 0                },
  { '6', Do_erase_application_firmware,        0                },
  { '7', Do_erase_application_NV_settings,     0                },
  { '8', 0,                                 (void *)&MENU_TESTS },
  { 'R', 0,                                    0                },
  { 'M', 0,                                 (void *)&MENU_MAIN  },
  { 0 }
};

const T_VT100_Menu      MENU_ENGINEERING  =
{
  "Engineering menu",
  "\033[5C Engineering menu\r\n"
  "\033[5C <1> - Internal control and diagnostic\r\n"
  "\033[5C <2> - Bluetooth control\r\n"
  "\033[5C <3> - Flash protection control\r\n"
  "\033[5C <4> - SD card control\r\n"
  "\033[5C <5> - Returm default bootloader settings\r\n"
  "\033[5C <6> - Erase application firmware\r\n"
  "\033[5C <7> - Erase application NV settings\r\n"
  "\033[5C <8> - Tests\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_ENGINEERING_ITEMS,
};


const T_VT100_Menu_item MENU_MANAGEMENT_ITEMS[] =
{
  { '1', Do_date_time_set                , 0    },
  { '3', Do_Copy_Module_Settings_to_File , 0    },
  { 'R', 0, 0 },
  { 'M', 0, (void *)&MENU_MAIN },
  { 0 }
};


const T_VT100_Menu      MENU_MANAGEMENT  =
{
  "Management menu",
  "\033[5C Management menu\r\n"
  "\033[5C <1> - Setting the date and time\r\n"
  "\033[5C <3> - Save parameters to file\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_MANAGEMENT_ITEMS,
};

//-------------------------------------------------------------------------------------
const T_VT100_Menu_item MENU_MAIN_ITEMS[] =
{
  { '1', Do_MOD_Params_editor           , (void *)&MENU_PARAMETERS    },
  { '2', 0                              , (void *)&MENU_MANAGEMENT    },
  { '3', Do_show_event_log              , 0                           },
  { '4', Do_Reset                       , 0                           },
  { '5', 0                              , (void *)&MENU_ENGINEERING   },
  { 'R', 0                              , 0                           },
  { 'M', 0                              , (void *)&MENU_MAIN          },
  { 0                                                 }
};

const T_VT100_Menu      MENU_MAIN             =
{
  "Main menu",
  "\033[5C Main menu\r\n"
  "\033[5C <1> - Parameters\r\n"
  "\033[5C <2> - Management menu\r\n"
  "\033[5C <3> - Log\r\n"
  "\033[5C <4> - Reset\r\n",
  MENU_MAIN_ITEMS,
};

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Set_monitor_func(void (*func)(unsigned char))
{
  GET_MCBL;

  mcbl->Monitor_func = func;
}



/*-------------------------------------------------------------------------------------------------------------
  Вывести на экран текущее меню
-------------------------------------------------------------------------------------------------------------*/
void Display_menu(void)
{
  GET_MCBL;

  uint8_t *str;

  MPRINTF(VT100_CLEAR_AND_HOME);

  if (mcbl->menu_trace[mcbl->menu_nesting] == 0) return;

  str = (uint8_t *)ivar.product_name;
  // Вывод заголовка меню
  //VT100_send_str_to_pos((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_header, 1, Find_str_center((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_header));
  VT100_send_str_to_pos((uint8_t *)str, 1, VT100_find_str_center((uint8_t *)str));
  VT100_send_str_to_pos(DASH_LINE, 2, 0);
  // Вывод строки содержимого меню
  VT100_send_str_to_pos((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_body, 3, 0);
  MPRINTF("\r\n");
  MPRINTF(DASH_LINE);

}
/*-------------------------------------------------------------------------------------------------------------
  Поиск в текущем меню пункта вызываемого передаваемым кодом
  Параметры:
    b - код команды вазывающей пункт меню
  Возвращает:
    Указатель на соответствующий пункт в текущем меню
-------------------------------------------------------------------------------------------------------------*/
int32_t Lookup_menu_item(T_VT100_Menu_item **item, uint8_t b)
{
  int16_t           i;
  GET_MCBL;

  if (isalpha(b) != 0) b = toupper(b);

  i = 0;
  do
  {
    *item = (T_VT100_Menu_item *)mcbl->menu_trace[mcbl->menu_nesting]->menu_items + i;
    if ((*item)->but == b) return (1);
    if ((*item)->but == 0) break;
    i++;
  }while (1);

  return (0);
}




/*-------------------------------------------------------------------------------------------------------------
  Поиск пункта меню по коду вызова (в текущем меню)
  и выполнение соответствующей ему функции
  Параметры:
    b - код символа введенного с клавиатуры и вазывающего пункт меню

-------------------------------------------------------------------------------------------------------------*/
void Menu_press_key_handler(uint8_t b)
{
  T_VT100_Menu_item *menu_item;
  GET_MCBL;

  // Ищем запись в списке которой соответствует заданный символ в переменной b
  if (Lookup_menu_item(&menu_item, b) != 0)
  {
    // Нашли соответствующий пункт меню
    if (menu_item->psubmenu != 0)
    {
      // Если присутствует субменю, то вывести его

      if ((T_VT100_Menu *)menu_item->psubmenu == &MENU_ENGINEERING)
      {
        if (Enter_special_code() != RES_OK)
        {
          Display_menu();
          return;
        }
      }

      mcbl->menu_nesting++;
      mcbl->menu_trace[mcbl->menu_nesting] = (T_VT100_Menu *)menu_item->psubmenu;

      Display_menu();
      // Если есть функция у пункта меню, то передать ей обработчик нажатий в главном цикле и выполнить функцию.
      if (menu_item->func != 0)
      {
        mcbl->Monitor_func = (T_menu_func)(menu_item->func); // Установить обработчик нажатий главного цикла на функцию из пункта меню
        menu_item->func(0);                                  // Выполнить саму функцию меню
      }
    }
    else
    {
      if (menu_item->func == 0)
      {
        // Если нет ни субменю ни функции, значит это пункт возврата в предыдущее меню
        // Управление остается в главном цикле у обработчика по умолчанию
        Return_to_prev_menu();
        Display_menu();
      }
      else
      {
        // Если у пункта нет своего меню, то перейти очистить экран и перейти к выполению  функции выбранного пункта
        MPRINTF(VT100_CLEAR_AND_HOME);
        menu_item->func(0);
        // Управление возвращается в главный цикл обработчику по умолчанию
        Display_menu();
      }
    }

  }
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Goto_main_menu(void)
{
  GET_MCBL;

  mcbl->menu_nesting = 0;
  mcbl->menu_trace[mcbl->menu_nesting] = (T_VT100_Menu *)&MENU_MAIN;
  Display_menu();
  mcbl->Monitor_func = Menu_press_key_handler; // Назначение функции
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Return_to_prev_menu(void)
{
  GET_MCBL;

  if (mcbl->menu_nesting > 0)
  {
    mcbl->menu_trace[mcbl->menu_nesting] = 0;
    mcbl->menu_nesting--;
  }
  mcbl->Monitor_func = Menu_press_key_handler; // Назначение функции
}

/*-----------------------------------------------------------------------------------------------------
  Установка даты и времени
-----------------------------------------------------------------------------------------------------*/
static void Do_date_time_set(uint8_t keycode)
{
  unsigned char      i, k, b;
  uint8_t            buf[EDSTLEN];
  rtc_time_t         rt_time = {0};
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);

  VT100_send_str_to_pos("SYSTEM TIME SETTING", 1, 30);
  VT100_send_str_to_pos("\033[5C <M> - Display main menu, <Esc> - Exit \r\n", 2, 10);
  VT100_send_str_to_pos("Print in form [YY.MM.DD HH.MM.SS]:  .  .     :  :  ", SCR_ITEMS_VERT_OFFS, 1);

  mcbl->beg_pos = 35;
  k = 0;
  mcbl->curr_pos = mcbl->beg_pos;
  VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
  MPRINTF((char *)VT100_CURSOR_ON);

  for (i = 0; i < EDSTLEN; i++) buf[i] = 0;

  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case VT100_BCKSP:  // Back Space
        if (mcbl->curr_pos > mcbl->beg_pos)
        {
          mcbl->curr_pos--;
          k--;
          switch (k)
          {
          case 2:
          case 5:
          case 8:
          case 11:
          case 14:
            k--;
            mcbl->curr_pos--;
            break;
          }

          VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          MPRINTF((char *)" ");
          VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          buf[k] = 0;
        }
        break;
      case VT100_DEL:  // DEL
        mcbl->curr_pos = mcbl->beg_pos;
        k = 0;
        for (i = 0; i < EDSTLEN; i++) buf[i] = 0;
        VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->beg_pos);
        MPRINTF((char *)"  .  .     :  :  ");
        VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->beg_pos);
        break;
      case VT100_ESC:  // ESC
        MPRINTF((char *)VT100_CURSOR_OFF);
        return;
      case 'M':  //
      case 'm':  //
        MPRINTF((char *)VT100_CURSOR_OFF);
        return;

      case VT100_CR:  // Enter
        MPRINTF((char *)VT100_CURSOR_OFF);

        rt_time.tm_year  = BCD2ToBYTE((buf[0] << 4) + buf[1]) + 2000 - 1900;
        rt_time.tm_mon   = BCD2ToBYTE((buf[3] << 4) + buf[4]) - 1;
        rt_time.tm_mday  = BCD2ToBYTE((buf[6] << 4) + buf[7]);
        rt_time.tm_hour  = BCD2ToBYTE((buf[9] << 4) + buf[10]);
        rt_time.tm_min   = BCD2ToBYTE((buf[12] << 4) + buf[13]);
        rt_time.tm_sec   = BCD2ToBYTE((buf[15] << 4) + buf[16]);


        RTC_set_system_DateTime(&rt_time);
        return;
      default:
        if (isdigit(b))
        {
          if (k < EDSTLEN)
          {
            uint8_t str[2];
            str[0] = b;
            str[1] = 0;
            MPRINTF((char *)str);
            buf[k] = b - 0x30;
            mcbl->curr_pos++;
            k++;
            switch (k)
            {
            case 2:
            case 5:
            case 8:
            case 11:
            case 14:
              k++;
              mcbl->curr_pos++;
              break;
            }
            VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          }
        }
        break;

      } // switch
    }
  }while (1);
}

/*-------------------------------------------------------------------------------------------
  Редактирование параметров модуля
  Функция вызываеся один раз при входе в режим редактирования параметров
---------------------------------------------------------------------------------------------*/
void Do_MOD_Params_editor(uint8_t keycode)
{
  GET_MCBL;

  mcbl->p_pinst = Get_mod_params_instance();
  if (mcbl->p_pinst == 0) return;
  mcbl->ptype = BOOTL_PARAMS;
  mcbl->current_level = MAIN_PARAMS_ROOT;
  Show_parameters_menu();                               // Показать меню параметров
  Set_monitor_func(Params_editor_press_key_handler);    // Переназначение обработчика нажатий в главном цикле монитора
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_show_event_log(uint8_t keycode)
{
  App_Log_monitor();
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void Do_Copy_Module_Settings_to_File(uint8_t keycode)
{
  uint32_t res;
  GET_MCBL;

  ivar.en_formated_settings = 1;
  ivar.en_compress_settins  = 0;
  res = Save_settings_to(&ivar_inst, MEDIA_TYPE_FILE, PARAMS_MODULE_JSON_FILE_NAME, BOOTL_PARAMS);
  if (res == RES_OK)
  {
    MPRINTF(VT100_CLR_LINE"Module parameters saved to file %s successfully!\n\r\n\r", PARAMS_MODULE_JSON_FILE_NAME);
  }
  else
  {
    MPRINTF(VT100_CLR_LINE"Module parameters don't saved. An error has occurred!\n\r\n\r");
  }
  Wait_ms(2000);
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void Do_return_defaul_bootloader_settings(uint8_t keycode)
{
  uint8_t   b;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF("Press Y if you want to return default settings\r\n");

  do
  {
    if (WAIT_CHAR(&b,  ms_to_ticks(10000)) != RES_OK)
    {
      return;
    }

    if ((b == 'Y') || (b == 'y'))
    {
      Return_def_params(&ivar_inst);
      if (Save_settings_to(&ivar_inst, MEDIA_TYPE_DATAFLASH, 0, BOOTL_PARAMS) == RES_OK)
      {
        MPRINTF("\r\n Settings restored successfully!");
        Wait_ms(2000);
        return;
      }
      else
      {
        MPRINTF("\r\n Settings restoring fault!");
        Wait_ms(2000);
        return;
      }
    }
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void     Do_erase_application_firmware(uint8_t keycode)
{
  uint32_t  res;
  uint8_t   b;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF("Press Y if you want to erase application firmware\r\n");

  do
  {
    if (WAIT_CHAR(&b,  ms_to_ticks(10000)) != RES_OK)
    {
      return;
    }

    if ((b == 'Y') || (b == 'y'))
    {
      VT100_set_cursor_pos(5, 0);
      MPRINTF(VT100_CLL_FM_CRSR);
      MPRINTF(" Wait...");
      Wait_ms(10);

      res = Erase_firmware_area();
      VT100_set_cursor_pos(5, 0);
      MPRINTF(VT100_CLL_FM_CRSR);

      if (res== RES_OK)
      {
        MPRINTF("Erasing done successfully!");
        Wait_ms(2000);
        return;
      }
      else
      {
        MPRINTF("Erasing fail!");
        Wait_ms(2000);
        return;
      }
    }
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void     Do_erase_application_NV_settings(uint8_t keycode)
{
  uint32_t  res;
  uint8_t   b;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF("Press Y if you want to erase application NV settings\r\n");

  do
  {
    if (WAIT_CHAR(&b,  ms_to_ticks(10000)) != RES_OK)
    {
      return;
    }

    if ((b == 'Y') || (b == 'y'))
    {
      VT100_set_cursor_pos(5, 0);
      MPRINTF(VT100_CLL_FM_CRSR);
      MPRINTF(" Wait...");
      Wait_ms(10);

      res = Clear_app_DataFlash();
      VT100_set_cursor_pos(5, 0);
      MPRINTF(VT100_CLL_FM_CRSR);

      if (res== RES_OK)
      {
        MPRINTF("Erasing done successfully!");
        Wait_ms(2000);
        return;
      }
      else
      {
        MPRINTF("Erasing fail!");
        Wait_ms(2000);
        return;
      }
    }
  }while (1);

}


/*------------------------------------------------------------------------------
 Сброс системы
 ------------------------------------------------------------------------------*/
static void Do_Reset(uint8_t keycode)
{
  Reset_SoC();
}

