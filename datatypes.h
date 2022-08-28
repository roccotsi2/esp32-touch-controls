//#pragma pack(1)
typedef struct buttonData {
   uint8_t id;
   Rect_t area;
   char *text;
} __attribute__ ((packed)) ButtonData;

//#pragma pack(1)
typedef struct listBoxData {
   uint8_t id;
   Rect_t area;
   char *text;
   char elements[10][30];
   int elementCount;
   int elementsPerPage;
   int elementHeight;
   int currentPageNo;
   uint8_t buttonIdsElements[10];
   int buttonIdUp;
   int buttonIdDown;
} __attribute__ ((packed)) ListBoxData;
