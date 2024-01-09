
#pragma once

struct Comparable {
    virtual ~Comparable() = default;

    friend bool operator<(const Comparable &cmp1, const Comparable &cmp2)
    {
        return cmp1.hashCode() < cmp2.hashCode();
    }

    virtual size_t hashCode() const = 0;
};
