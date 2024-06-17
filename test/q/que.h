/*********************************************************************
*   Name          : QUE.H
*   Description   : Header Module Consists of the Que Functions
*   Version       : 1.00c
*   Date          : 06/09/99
*   Author        : LEO
*   Reference     : X
*********************************************************************/
#ifdef  QUE_GLOBAL
#define QUE_EXT
#else
#define QUE_EXT  extern
#endif

/*-------------------------------------------------------------------
- TYPEDEF
-------------------------------------------------------------------*/
typedef enum Q_Size
{
  BIT8 = 0,
  BIT16
}Queue_Size_Type;

typedef enum Q_Status
{
  Q_FULL,
  Q_EMPTY,
  Q_OK
}Queue_Status_Type;

/*-------------------------------------------------------------------
- STRUCTURES
-------------------------------------------------------------------*/
struct QUE
{
  unsigned char *data;
  unsigned char max;
  unsigned char quein;
  unsigned char queout;
};

/*-------------------------------------------------------------------
- PROTOTYPES
-------------------------------------------------------------------*/
void Que_Setup(struct QUE * const que, unsigned char *pdata, const unsigned char max);
QUE_EXT void Que_Clear(struct QUE *);
QUE_EXT Queue_Status_Type Que_Status(struct QUE *);
QUE_EXT void Que_Put(struct QUE *, unsigned char);
QUE_EXT unsigned char Que_Get(struct QUE *);
QUE_EXT unsigned char Que_Look(struct QUE *);
QUE_EXT void Que_Remove(struct QUE *);
/*-------------------------------------------------------------------
- GLOBAL DEFINITIONS
-------------------------------------------------------------------*/

