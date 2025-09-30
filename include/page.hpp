#ifndef INCLUDE_PAGE_HPP
#define INCLUDE_PAGE_HPP

#include <cstdio>

struct Page {
    size_t id;
    size_t size;
    char *data;

    static Page slow_get_page(size_t id) { return Page{id}; }

    Page() : id(0), size(0), data(nullptr) {}
    Page(size_t init_id) : id(init_id), size(0), data(nullptr) {}
};

#endif