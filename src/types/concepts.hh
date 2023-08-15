#pragma once

/*

Include to disable concepts to fix horrendous error messages

*/

#define ENABLE_CONCEPTS

#ifdef ENABLE_CONCEPTS
#define CONCEPT(Concept, Types) Concept Types
#else
#define CONCEPT(Concept, Types) typename Types
#endif