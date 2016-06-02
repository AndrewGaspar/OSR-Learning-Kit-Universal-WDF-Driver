#pragma once

#define INIT_CODE __declspec(code_seg("INIT"))
#define PAGED __declspec(code_seg("PAGE"))
#define NONPAGED __declspec(code_seg("NONPAGED"))

#define PASSIVE _IRQL_requires_same_ _IRQL_requires_max_(PASSIVE_LEVEL)