#pragma once

/*

Include to disable concepts to fix horrendous error messages

*/

#ifdef ENABLE_CONCEPTS
#define CONCEPT(Concept, Types) Concept Types
#else
#define CONCEPT(Concept, Types) typename Types
#endif