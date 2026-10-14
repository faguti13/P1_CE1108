#include "tokens.h"

#include <stdio.h>

const char *nombre_token(int token)
{
    switch (token) {
    case TK_ENT:         return "DataE";
    case TK_REAL:        return "DataR";
    case TK_DOBLE:       return "DataD";
    case TK_BOOL:        return "DataB";
    case TK_CARACT:      return "DataC";
    case TK_CAD:         return "DataSTR";
    case TK_VACIO:       return "DataV";
    case TK_LIST:        return "DataL";
    case TK_VERDADERO:   return "BoolV";
    case TK_FALSO:       return "BoolF";
    case TK_DEF:         return "DEF";
    case TK_LLAMAR:      return "CALL";
    case TK_RETORNO:     return "RTN";
    case TK_TERMINAR:    return "BRK";
    case TK_IMPORT:      return "IMP";
    case TK_SI:          return "IF";
    case TK_SINO:        return "ELS";
    case TK_MIENTRAS:    return "WHL";
    case TK_PARA:        return "FOR";
    case TK_IDENT:       return "IdVAR";
    case TK_ENTERO:      return "LitEnt";
    case TK_CADENA:      return "LitCad";
    case TK_CARACTER:    return "LitCar";
    case TK_IGUAL:       return "Op==";
    case TK_DISTINTO:    return "Op!=";
    case TK_MENOR_IGUAL: return "Op<=";
    case TK_MAYOR_IGUAL: return "Op>=";
    case TK_AND:         return "Op&&";
    case TK_OR:          return "Op||";
    case TK_MAS_IGUAL:   return "Op+=";
    case TK_MENOS_IGUAL: return "Op-=";
    case TK_ERROR:       return "ERROR";

    case '+': return "Op+";
    case '-': return "Op-";
    case '*': return "Op*";
    case '/': return "Op/";
    case '%': return "Op%";
    case '=': return "Op=";
    case '<': return "Op<";
    case '>': return "Op>";
    case '!': return "Op!";
    case '(': return "Del(";
    case ')': return "Del)";
    case '{': return "Del{";
    case '}': return "Del}";
    case '[': return "Del[";
    case ']': return "Del]";
    case ',': return "Del,";
    case ';': return "Del;";
    case '.': return "Del.";
    case ':': return "Del:";
    default:
        break;
    }

    {
        static char buf[16];
        if (token >= 32 && token < 127) {
            snprintf(buf, sizeof buf, "'%c'", token);
        } else {
            snprintf(buf, sizeof buf, "TK_%d", token);
        }
        return buf;
    }
}
