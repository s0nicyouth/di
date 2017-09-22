#ifndef INJECT_MARKERS_H_
#define INJECT_MARKERS_H_

namespace di {
#define str(s) #s
#define INJECT(name, ...) explicit name(di::DiMark&& m, __VA_ARGS__)

struct DiMark{};
}

#endif  // INJECT_MARKERS_H_