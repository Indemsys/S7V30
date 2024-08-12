// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2024-07-25
// 15:08:28
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


#define MAX_DEPTH   5
#define MAX_NODES   100


typedef struct T_Node
{
    uint32_t level_id;
    uint8_t *name;
    struct T_Node *children[MAX_NODES];
    int32_t child_count;
} T_Node;

static T_Node *g_nodes[MAX_NODES];
static int32_t g_node_count = 0;

/*-----------------------------------------------------------------------------------------------------


  \param level_id
  \param name

  \return T_Node*
-----------------------------------------------------------------------------------------------------*/
static T_Node* Find_or_Create_Node(uint32_t level_id, uint8_t *name)
{
  T_Node *node;

  for (int32_t i = 0; i < g_node_count; i++)
  {
    if (g_nodes[i]->level_id == level_id)
    {
      if (g_nodes[i]->name == NULL && name)
      {
        g_nodes[i]->name = (uint8_t *)App_malloc_pending(strlen((char *)name)+ 1, MS_TO_TICKS(100));
        if (g_nodes[i]->name == NULL) return NULL;
        strcpy((char *)g_nodes[i]->name, (char *)name);
      }
      return g_nodes[i];
    }
  }
  node = (T_Node *) App_malloc_pending(sizeof(T_Node), MS_TO_TICKS(100));
  if (node == NULL) return NULL;
  node->level_id = level_id;

  if (name)
  {
    node->name = (uint8_t *)App_malloc_pending(strlen((char *)name)+ 1,  MS_TO_TICKS(100));
    if (node->name == NULL) return NULL;
    strcpy((char *)node->name, (char *)name);
  }
  else
  {
    node->name = NULL;
  }
  node->child_count = 0;
  g_nodes[g_node_count++] = node;
  return node;
}

/*-----------------------------------------------------------------------------------------------------


  \param parent
  \param child
-----------------------------------------------------------------------------------------------------*/
static void Add_Child(T_Node *parent, T_Node *child)
{
  if (parent->child_count < MAX_NODES)
  {
    int32_t pos = 0;
    while (pos < parent->child_count && parent->children[pos]->level_id < child->level_id)
    {
      pos++;
    }
    if (pos < parent->child_count)
    {
      memmove(&parent->children[pos + 1],&parent->children[pos], sizeof(T_Node *) * (parent->child_count - pos));
    }
    parent->children[pos] = child;
    parent->child_count++;
  }
}


/*-----------------------------------------------------------------------------------------------------
  Ищем параметр с минимальным позиционным номером, но большим чем curr_pos_number

  \param p_pars
  \param sid

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t Find_minimal_id_params(const T_NV_parameters_instance  *p_pars, uint32_t level_id, int32_t *curr_pos_number)
{
  int32_t par_indx = -1;
  int32_t min_pos_number = 0x7FFFFFFF;

  for (uint32_t i = 0; i < p_pars->items_num; i++)
  {
    if (p_pars->items_array[i].parmnlev == level_id)
    {
      int32_t pos_number = (int32_t)p_pars->items_array[i].menu_pos;
      if (pos_number > *curr_pos_number)
      {
        if (pos_number < min_pos_number)
        {
          min_pos_number    = pos_number;
          *curr_pos_number = pos_number;
          par_indx   = i;
        }
      }
    }
  }
  return par_indx;
}

/*-----------------------------------------------------------------------------------------------------


  \param level_id
  \param buffer
  \param offset
-----------------------------------------------------------------------------------------------------*/
static void Print_params_list(const T_NV_parameters_instance  *p_pars, uint32_t level_id, char *buffer, uint32_t *offset)
{
  int32_t   start_pos = -1;
  uint32_t  first     = 1;
  int32_t   par_indx;
  uint32_t  sel_indx;
  char      tmp_str[128];

  // Создаем объект "Parameters" с массивом объектов параметров
  *offset += Print_to(buffer,*offset, "\"Parameters\":[");

  do
  {
    // Ищем параметры относящиеся к заданному уровню в порядке увеличения из позиции в меню
    par_indx = Find_minimal_id_params(p_pars, level_id,&start_pos);
    if (par_indx < 0) break;


    if (!first) *offset += Print_to(buffer,*offset, ",");
    else first = 0;
    // Создаем объект "Parameters" с массивом объектов параметров
    *offset += Print_to(buffer,*offset, "{");
    *offset += Print_to(buffer,*offset, "\"id\":%d,", p_pars->items_array[par_indx].menu_pos);
    *offset += Print_to(buffer,*offset, "\"name\":\"%s\",", p_pars->items_array[par_indx].var_name);
    *offset += Print_to(buffer,*offset, "\"description\":\"%s\",", p_pars->items_array[par_indx].var_description);
    switch (p_pars->items_array[par_indx].vartype)
    {
    case tint8u      :
      *offset += Print_to(buffer,*offset, "\"type\":\"uint8_t\",");
      *offset += Print_to(buffer,*offset, "\"default_value\":%d,", (uint32_t)p_pars->items_array[par_indx].defval);
      *offset += Print_to(buffer,*offset, "\"min_value\":%d,", (uint32_t)p_pars->items_array[par_indx].minval);
      *offset += Print_to(buffer,*offset, "\"max_value\":%d,", (uint32_t)p_pars->items_array[par_indx].maxval);
      *offset += Print_to(buffer,*offset, "\"value\":%d,",*((uint8_t *)p_pars->items_array[par_indx].val));
      break;
    case tint16u     :
      *offset += Print_to(buffer,*offset, "\"type\":\"uint16_t\",");
      *offset += Print_to(buffer,*offset, "\"default_value\":%d,", (uint32_t)p_pars->items_array[par_indx].defval);
      *offset += Print_to(buffer,*offset, "\"min_value\":%d,", (uint32_t)p_pars->items_array[par_indx].minval);
      *offset += Print_to(buffer,*offset, "\"max_value\":%d,", (uint32_t)p_pars->items_array[par_indx].maxval);
      *offset += Print_to(buffer,*offset, "\"value\":%d,",*((uint16_t *)p_pars->items_array[par_indx].val));
      break;
    case tint32u     :
      *offset += Print_to(buffer,*offset, "\"type\":\"uint32_t\",");
      *offset += Print_to(buffer,*offset, "\"default_value\":%d,", (uint32_t)p_pars->items_array[par_indx].defval);
      *offset += Print_to(buffer,*offset, "\"min_value\":%d,", (uint32_t)p_pars->items_array[par_indx].minval);
      *offset += Print_to(buffer,*offset, "\"max_value\":%d,", (uint32_t)p_pars->items_array[par_indx].maxval);
      *offset += Print_to(buffer,*offset, "\"value\":%d,",*((uint32_t *)p_pars->items_array[par_indx].val));
      break;
    case tfloat      :
      *offset += Print_to(buffer,*offset, "\"type\":\"float\",");
      *offset += Print_to(buffer,*offset, "\"default_value\":%e,", (double)p_pars->items_array[par_indx].defval);
      *offset += Print_to(buffer,*offset, "\"min_value\":%e,", (double)p_pars->items_array[par_indx].minval);
      *offset += Print_to(buffer,*offset, "\"max_value\":%e,", (double)p_pars->items_array[par_indx].maxval);
      *offset += Print_to(buffer,*offset, "\"value\":%e,",(double)*((float*)p_pars->items_array[par_indx].val));
      break;
    case tstring     :
      *offset += Print_to(buffer,*offset, "\"type\":\"string\",");
      *offset += Print_to(buffer,*offset, "\"default_value\":0,");
      *offset += Print_to(buffer,*offset, "\"min_value\":0,");
      *offset += Print_to(buffer,*offset, "\"max_value\":0,");
      *offset += Print_to(buffer,*offset, "\"value\":\"%s\",", (char *)p_pars->items_array[par_indx].val);
      break;
    case tint32s     :
      *offset += Print_to(buffer,*offset, "\"type\":\"int32_t\",");
      *offset += Print_to(buffer,*offset, "\"default_value\":%d,", (int32_t)p_pars->items_array[par_indx].defval);
      *offset += Print_to(buffer,*offset, "\"min_value\":%d,", (int32_t)p_pars->items_array[par_indx].minval);
      *offset += Print_to(buffer,*offset, "\"max_value\":%d,", (int32_t)p_pars->items_array[par_indx].maxval);
      *offset += Print_to(buffer,*offset, "\"value\":%d,",*((int32_t *)p_pars->items_array[par_indx].val));
      break;
    case tarrofdouble:
    case tarrofbyte  :
      break;
    }

    *offset += Print_to(buffer,*offset, "\"attr\":%d,", (uint32_t)p_pars->items_array[par_indx].attr);
    *offset += Print_to(buffer,*offset, "\"max_len\":%d,", (uint32_t)p_pars->items_array[par_indx].varlen);
    *offset += Print_to(buffer,*offset, "\"format\":\"%s\",", p_pars->items_array[par_indx].format);

    // Выводим селекторы
    sel_indx = p_pars->items_array[par_indx].selector_id;
    *offset += Print_to(buffer,*offset, "\"options_name\":\"%s\",", p_pars->selectors_array[sel_indx].name);
    *offset += Print_to(buffer,*offset, "\"options\":[");
    {
      for (uint32_t i = 0; i < p_pars->selectors_array[sel_indx].items_cnt; i++)
      {
        T_selector_items const *si = p_pars->selectors_array[sel_indx].items_list;
        if (i > 0) *offset += Print_to(buffer,*offset, ",");
        *offset += Print_to(buffer,*offset, "{");
        *offset += Print_to(buffer,*offset, "\"v\":%d,", si[i].val);
        Trim_str((char const *)si[i].caption, tmp_str, sizeof(tmp_str));
        *offset += Print_to(buffer,*offset, "\"n\":\"%s\"", tmp_str);
        *offset += Print_to(buffer,*offset, "}");
      }
    }
    *offset += Print_to(buffer,*offset, "]");

    // Закрываем массив параметров
    *offset += Print_to(buffer,*offset, "}");


  } while (1);

  // Создаем объект "Parameters" с массивом объектов параметров
  *offset += Print_to(buffer,*offset, "],");

}
/*-----------------------------------------------------------------------------------------------------


  \param node
  \param buffer

  \return char*
-----------------------------------------------------------------------------------------------------*/
static uint32_t Print_levels_tree(const T_NV_parameters_instance  *p_pars, T_Node *node, char *buffer, uint32_t *offset)
{
  char *nodeName;

  // Здесь провереям, разрешен ли уровень. Если нет, то не печатаем
  for (uint32_t i=0;i<PARMNU_ITEM_NUM;i++)
  {
    if (node->level_id == parmenu[i].currlev)
    {
      if (parmenu[i].visible == 0) return RES_ERROR;
    }
  }


  if (node->name)
  {
    nodeName = (char *)node->name;
  }
  else
  {
    nodeName = "(no name)";
  }

  *offset += Print_to(buffer,*offset, "{\"NodeID\":%u,\"Name\":\"%s\",", node->level_id, nodeName);

  // Здесь выводим информацию о всех параметрах принадлежащих этому уровню
  Print_params_list(p_pars, node->level_id, buffer, offset);
  *offset += Print_to(buffer,*offset, "\"Children\":[");

  for (int32_t i = 0; i < node->child_count; i++)
  {
    if (Print_levels_tree(p_pars, node->children[i], buffer, offset)==RES_OK)
    {
      if (i < node->child_count - 1)
      {
        *offset += Print_to(buffer,*offset, ",");
      }
    }
  }

  *offset += Print_to(buffer,*offset, "]}");
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param p_pars

  \return uint8_t*
-----------------------------------------------------------------------------------------------------*/
char* ParamsSchema_serialize_to_buff(const T_NV_parameters_instance  *p_pars, uint32_t *sz_ptr)
{
  char            *buffer   = NULL;
  uint32_t         offset;
  T_Node         *parent;
  T_Node         *child;
  T_Node         *root;

  const T_parmenu *records = p_pars->menu_items_array;
  int32_t          num_records = p_pars->menu_items_num;

  g_node_count = 0;
  memset(g_nodes, 0, sizeof(g_nodes));

  for (int32_t i = 0; i < num_records; i++)
  {
    parent = Find_or_Create_Node(records[i].prevlev, NULL);
    if (parent == NULL) goto exit_;
    child = Find_or_Create_Node(records[i].currlev, (uint8_t *)records[i].name);
    if (child == NULL) goto exit_;
    Add_Child(parent, child);
  }

  root = Find_or_Create_Node(1, NULL);  // Ensure we have a root at level 1
  if (root == NULL) goto exit_;

  offset = 0;
  Print_levels_tree(p_pars, root, NULL,&offset);

  buffer = App_malloc_pending(offset + 1, MS_TO_TICKS(100));  // Initialize an empty buffer
  if (buffer == NULL) goto exit_;

  offset = 0;
  Print_levels_tree(p_pars, root, buffer,&offset);
  *sz_ptr = offset;

exit_:
  for (int32_t i = 0; i < g_node_count; i++)
  {
    if (g_nodes[i] != 0) App_free(g_nodes[i]->name);
    App_free(g_nodes[i]);
  }

  return buffer;
}


/*-----------------------------------------------------------------------------------------------------
  Описание структуры  T_NV_parameters_instance
  uint32_t                  items_num;          - Количество параметров
  const T_NV_parameters    *items_array;        - Указатель на массив параметров
  uint32_t                  menu_items_num;     - Количество узлов в дереве параметров
  const T_parmenu          *menu_items_array;   - Указатель на массив описания узлов дерева паретров
  uint32_t                  selectors_num;      - Количество записей в массиве селекторов
  const T_selectors_list   *selectors_array;    - Указатель на массив селекторов



  \param p_pars
-----------------------------------------------------------------------------------------------------*/
uint32_t ParamsSchema_serialize_to_file(const T_NV_parameters_instance  *p_pars, const char *file_name)
{
  uint32_t         status   = RES_OK;
  char            *buffer   = NULL;
  uint32_t         buff_sz;
  FX_FILE         *file_ptr = NULL;

  buffer = ParamsSchema_serialize_to_buff(p_pars,&buff_sz);
  if (buffer == NULL)
  {
    APPLOG("Error");
    return RES_ERROR;
  }

  file_ptr = App_malloc_pending(sizeof(FX_FILE), MS_TO_TICKS(100));
  if (file_ptr != NULL)
  {
    uint32_t res;

    res = Recreate_file_for_write(file_ptr, (CHAR *)file_name);
    if (res != FX_SUCCESS)
    {
      APPLOG("Error %d",res);
      status = RES_ERROR;
    }
    else
    {
      if (fx_file_write(file_ptr, buffer, buff_sz) != FX_SUCCESS)
      {
        status = RES_ERROR;
      }
      fx_file_close(file_ptr);
    }
    App_free(file_ptr);
  }
  else
  {
    status = RES_ERROR;
  }

  App_free(buffer);

  return status;
}




