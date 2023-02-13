/* Atributul este folosit pentru a anunta compilatorul sa nu alinieze structura */
__attribute__((packed))
/* DELIM | DATE | DELIM */
struct Frame {
    char frame_delim_start[2]; /* DEL STX */
    char payload[30];  /* Datele pe care vrem sa le transmitem */
    char frame_delim_end[2]; /* DEL ETX */
};

