/**
 * @file RPM_Version.h
 * @author Hello007
 * @brief Library version macro header.
 * @version 0.1
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_VERSION_H
#define __RPM_VERSION_H

/**
 * @brief Current version of the Relocatable Program Module library and supported binary formats.
 */
#define LIBRPM_VERSION 11 //libRPM v0.11

/**
 *  === RELOCATABLE PROGRAM MODULE LIBRARY - VERSION HISTORY ===
 * 
 *  - v0.0  : Initial version of the format.
 *  - v0.1  : Added symbol length field.
 *  - v0.2  : Added product name and version metadata fields. THIS VERSION HAS BEEN TERMINATED!
 *  - v0.3  : Use the string table to write symbol names. Makes structs more easily deserializable and opens the door for possible string compression.
 *          : Reduce the size of the symbol table by writing string pointers and symbol sizes as shorts.
 *          : Write essential header fields into a separately referenced INFO section. This eliminates the need for reserved values in the RPM footer.
 *          : Implements the capability to add arbitrary user data to an RPM.
 *  - v0.4  : Omits empty sections completely.
 *  - v0.5  : Format redesigned for aligned memory safety.
 *  - v0.6  : Pre-generated lists of relocation and symbol extern modules.
 *  - v0.7  : Code segment no longer required to be at the start of the file (0x20-byte prolog).
 *          : Separate internal / external relocation tables.
 *  - v0.8  : Import and export symbol hashing for blazing fast lookup.
 *  - v0.9  : Separate relocation list for imported symbols (allows loading libraries from within module).
 *  - v0.10 : Remove external relocation sources (never used), shrink rpm::Relocation size to 12 bytes.
 *  - v0.11 : Add BSS support / module expansion, header fields now relative to start of DLXH.
 */

#endif