#pragma once

namespace System {
    namespace Detail {
        using buffer = std::aligned_storage_t<sizeof(void*) * 2, alignof(void*)>;

        template <class T>
        inline constexpr bool is_small_object_v =
            sizeof(T) <= sizeof(buffer) && alignof(buffer) % alignof(T) == 0 &&
            std::is_nothrow_move_constructible_v<T>;

        union storage {
            void* ptr_ = nullptr;
            buffer buf_;
        };

        enum class action { destroy, move };

        template <class R, class... ArgTypes>
        struct handler_traits {
            template <class Derived>
            struct handler_base {
                static void handle(action act, storage* current, storage* other = nullptr) {
                    switch (act) {
                    case (action::destroy):
                        Derived::destroy(*current);
                        break;
                    case (action::move):
                        Derived::move(*current, *other);
                        break;
                    }
                }
            };

            template <class T>
            struct small_handler : handler_base<small_handler<T>> {
                template <class... Args>
                static void create(storage& s, Args&&... args) {
                    new (static_cast<void*>(&s.buf_)) T(std::forward<Args>(args)...);
                }

                static void destroy(storage& s) noexcept {
                    T& value = *static_cast<T*>(static_cast<void*>(&s.buf_));
                    value.~T();
                }

                static void move(storage& dst, storage& src) noexcept {
                    create(dst, std::move(*static_cast<T*>(static_cast<void*>(&src.buf_))));
                    destroy(src);
                }

                static R call(const storage& s, ArgTypes... args) {
                    return std::invoke(
                        *static_cast<T*>(static_cast<void*>(&const_cast<storage&>(s).buf_)),
                        std::forward<ArgTypes>(args)...);
                }
            };

            template <class T>
            struct large_handler : handler_base<large_handler<T>> {
                template <class... Args>
                static void create(storage& s, Args&&... args) {
                    s.ptr_ = new T(std::forward<Args>(args)...);
                }

                static void destroy(storage& s) noexcept { delete static_cast<T*>(s.ptr_); }

                static void move(storage& dst, storage& src) noexcept {
                    dst.ptr_ = src.ptr_;
                }

                static R call(const storage& s, ArgTypes... args) {
                    return std::invoke(*static_cast<T*>(s.ptr_),
                        std::forward<ArgTypes>(args)...);
                }
            };

            template <class T>
            using handler = std::conditional_t<is_small_object_v<T>, small_handler<T>,
                large_handler<T>>;
        };

        template <class R, class C, class... ArgTypes>
        struct handler_member_traits {
            template <class Derived>
            struct handler_base {
                static void handle(action act, storage* current, storage* other = nullptr) {
                    switch (act) {
                    case (action::destroy):
                        Derived::destroy(*current);
                        break;
                    case (action::move):
                        Derived::move(*current, *other);
                        break;
                    }
                }
            };

            template <class T>
            struct small_handler : handler_base<small_handler<T>> {
                template <class... Args>
                static void create(storage& s, Args&&... args) {
                    new (static_cast<void*>(&s.buf_)) T(std::forward<Args>(args)...);
                }

                static void destroy(storage& s) noexcept {
                    T& value = *static_cast<T*>(static_cast<void*>(&s.buf_));
                    value.~T();
                }

                static void move(storage& dst, storage& src) noexcept {
                    create(dst, std::move(*static_cast<T*>(static_cast<void*>(&src.buf_))));
                    destroy(src);
                }

                static R call(const storage& s, C* owner, ArgTypes... args) {
                    return std::invoke(
                        *static_cast<T*>(static_cast<void*>(&const_cast<storage&>(s).buf_)),
                        owner,
                        std::forward<ArgTypes>(args)...);
                }
            };

            template <class T>
            struct large_handler : handler_base<large_handler<T>> {
                template <class... Args>
                static void create(storage& s, Args&&... args) {
                    s.ptr_ = new T(std::forward<Args>(args)...);
                }

                static void destroy(storage& s) noexcept { delete static_cast<T*>(s.ptr_); }

                static void move(storage& dst, storage& src) noexcept {
                    dst.ptr_ = src.ptr_;
                }

                static R call(const storage& s, C* owner, ArgTypes... args) {
                    return std::invoke(*static_cast<T*>(s.ptr_),
                        owner,
                        std::forward<ArgTypes>(args)...);
                }
            };

            template <class T>
            using handler = std::conditional_t<is_small_object_v<T>, small_handler<T>,
                large_handler<T>>;
        };

        template <class T>
        struct is_in_place_type : std::false_type {};

        template <class T>
        struct is_in_place_type<std::in_place_type_t<T>> : std::true_type {};

        template <class T>
        inline constexpr auto is_in_place_type_v = is_in_place_type<T>::value;

        template <class R, bool is_noexcept, class... ArgTypes>
        class AnyInvocableImpl {
            template <class T>
            using handler =
                typename Detail::handler_traits<R, ArgTypes...>::template handler<T>;

            using storage = Detail::storage;
            using action = Detail::action;
            using handle_func = void (*)(Detail::action, Detail::storage*,
                Detail::storage*);
            using call_func = R(*)(const Detail::storage&, ArgTypes...);

        public:
            using result_type = R;

            AnyInvocableImpl() noexcept = default;
            AnyInvocableImpl(std::nullptr_t) noexcept {}
            AnyInvocableImpl(AnyInvocableImpl&& rhs) noexcept {
                if (rhs.handle_) {
                    handle_ = rhs.handle_;
                    handle_(action::move, &storage_, &rhs.storage_);
                    call_ = rhs.call_;
                    rhs.handle_ = nullptr;
                }
            }

            AnyInvocableImpl& operator=(AnyInvocableImpl&& rhs) noexcept {
                AnyInvocableImpl{ std::move(rhs) }.swap(*this);
                return *this;
            }
            AnyInvocableImpl& operator=(std::nullptr_t) noexcept {
                destroy();
                return *this;
            }

            ~AnyInvocableImpl() { destroy(); }

            void swap(AnyInvocableImpl& rhs) noexcept {
                if (handle_) {
                    if (rhs.handle_) {
                        storage tmp;
                        handle_(action::move, &tmp, &storage_);
                        rhs.handle_(action::move, &storage_, &rhs.storage_);
                        handle_(action::move, &rhs.storage_, &tmp);
                        std::swap(handle_, rhs.handle_);
                        std::swap(call_, rhs.call_);
                    }
                    else {
                        rhs.swap(*this);
                    }
                }
                else if (rhs.handle_) {
                    rhs.handle_(action::move, &storage_, &rhs.storage_);
                    handle_ = rhs.handle_;
                    call_ = rhs.call_;
                    rhs.handle_ = nullptr;
                }
            }

            explicit operator bool() const noexcept { return handle_ != nullptr; }

        protected:
            template <class F, class... Args>
            void create(Args&&... args) {
                using hdl = handler<F>;
                hdl::create(storage_, std::forward<Args>(args)...);
                handle_ = &hdl::handle;
                call_ = &hdl::call;
            }

            void destroy() noexcept {
                if (handle_) {
                    handle_(action::destroy, &storage_, nullptr);
                    handle_ = nullptr;
                }
            }

            R call(ArgTypes... args) const noexcept(is_noexcept) {
                return call_(storage_, std::forward<ArgTypes>(args)...);
            }

            friend bool operator==(const AnyInvocableImpl& f, std::nullptr_t) noexcept {
                return !f;
            }
            friend bool operator==(std::nullptr_t, const AnyInvocableImpl& f) noexcept {
                return !f;
            }
            friend bool operator!=(const AnyInvocableImpl& f, std::nullptr_t) noexcept {
                return static_cast<bool>(f);
            }
            friend bool operator!=(std::nullptr_t, const AnyInvocableImpl& f) noexcept {
                return static_cast<bool>(f);
            }

            friend void swap(AnyInvocableImpl& lhs, AnyInvocableImpl& rhs) noexcept {
                lhs.swap(rhs);
            }

        private:
            storage storage_;
            handle_func handle_ = nullptr;
            call_func call_ = nullptr;
        };

        template <class T>
        using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

        template <class AI, class F, bool noex, class R, class FCall, class... ArgTypes>
        using can_convert = std::conjunction<
            std::negation<std::is_same<remove_cvref_t<F>, AI>>,
            std::negation<Detail::is_in_place_type<remove_cvref_t<F>>>,
            std::is_invocable_r<R, FCall, ArgTypes...>,
            std::bool_constant<(!noex ||
                std::is_nothrow_invocable_r_v<R, FCall, ArgTypes...>)>,
            std::is_constructible<std::decay_t<F>, F>>;

        template <class R, typename C, bool is_noexcept, class... ArgTypes>
        class AnyInvocableMemberImpl {
            template <class T>
            using handler =
                typename Detail::handler_member_traits<R, C, ArgTypes...>::template handler<T>;

            using storage = Detail::storage;
            using action = Detail::action;
            using handle_func = void(*)(Detail::action, Detail::storage*,
                Detail::storage*);
            using call_func = R(*)(const Detail::storage&, C*, ArgTypes...);

        public:
            using result_type = R;

            AnyInvocableMemberImpl() noexcept = default;
            AnyInvocableMemberImpl(std::nullptr_t) noexcept {}
            AnyInvocableMemberImpl(AnyInvocableMemberImpl&& rhs) noexcept {
                if (rhs.handle_) {
                    handle_ = rhs.handle_;
                    handle_(action::move, &storage_, &rhs.storage_);
                    call_ = rhs.call_;
                    rhs.handle_ = nullptr;
                }
            }

            AnyInvocableMemberImpl& operator=(AnyInvocableMemberImpl&& rhs) noexcept {
                AnyInvocableMemberImpl{ std::move(rhs) }.swap(*this);
                return *this;
            }
            AnyInvocableMemberImpl& operator=(std::nullptr_t) noexcept {
                destroy();
                return *this;
            }

            ~AnyInvocableMemberImpl() { destroy(); }

            void swap(AnyInvocableMemberImpl& rhs) noexcept {
                if (handle_) {
                    if (rhs.handle_) {
                        storage tmp;
                        handle_(action::move, &tmp, &storage_);
                        rhs.handle_(action::move, &storage_, &rhs.storage_);
                        handle_(action::move, &rhs.storage_, &tmp);
                        std::swap(handle_, rhs.handle_);
                        std::swap(call_, rhs.call_);
                    }
                    else {
                        rhs.swap(*this);
                    }
                }
                else if (rhs.handle_) {
                    rhs.handle_(action::move, &storage_, &rhs.storage_);
                    handle_ = rhs.handle_;
                    call_ = rhs.call_;
                    rhs.handle_ = nullptr;
                }
            }

            explicit operator bool() const noexcept { return handle_ != nullptr; }

        protected:
            template <class F, class... Args>
            void create(Args&&... args) {
                using hdl = handler<F>;
                hdl::create(storage_, std::forward<Args>(args)...);
                handle_ = &hdl::handle;
                call_ = &hdl::call;
            }

            void destroy() noexcept {
                if (handle_) {
                    handle_(action::destroy, &storage_, nullptr);
                    handle_ = nullptr;
                }
            }

            R call(C* owner, ArgTypes... args) const noexcept(is_noexcept) {
                return call_(storage_, owner, std::forward<ArgTypes>(args)...);
            }

            friend bool operator==(const AnyInvocableMemberImpl& f, std::nullptr_t) noexcept {
                return !f;
            }
            friend bool operator==(std::nullptr_t, const AnyInvocableMemberImpl& f) noexcept {
                return !f;
            }
            friend bool operator!=(const AnyInvocableMemberImpl& f, std::nullptr_t) noexcept {
                return static_cast<bool>(f);
            }
            friend bool operator!=(std::nullptr_t, const AnyInvocableMemberImpl& f) noexcept {
                return static_cast<bool>(f);
            }

            friend void swap(AnyInvocableMemberImpl& lhs, AnyInvocableMemberImpl& rhs) noexcept {
                lhs.swap(rhs);
            }

        private:
            storage storage_;
            handle_func handle_ = nullptr;
            call_func call_;
        };

        template <class AI, class F, bool noex, class R, class FCall, class T, class... ArgTypes >
        using can_convert_mem = std::conjunction<
            std::negation<std::is_same<remove_cvref_t<F>, AI>>,
            std::negation<Detail::is_in_place_type<remove_cvref_t<F>>>,
            std::is_invocable_r<R, FCall, T*, ArgTypes...>,
            std::bool_constant<(!noex ||
                std::is_nothrow_invocable_r_v<R, FCall, T*, ArgTypes...>)>,
            std::is_constructible<std::decay_t<F>, F>>;

        template <class Signature>
        class AnyInvocable;

#define __SYSTEM_ANY_INVOCABLE(cv, ref, noex, inv_quals)                        \
  template <class _R, class... _ArgTypes>                                        \
  class AnyInvocable<_R(_ArgTypes...) cv ref noexcept(noex)> final        \
      : public AnyInvocableImpl<_R, noex, _ArgTypes...> {          \
    using BaseType = AnyInvocableImpl<_R, noex, _ArgTypes...>;    \
                                                                               \
   public:                                                                     \
    using BaseType::BaseType;                                                \
    using ArgsType = std::tuple<_ArgTypes...>;                             \
    using ResultType = _R;                                                \
                                                                               \
    template <                                                                 \
        class F,                                                               \
        class = std::enable_if_t<can_convert<                      \
            AnyInvocable, F, noex, _R, F inv_quals, _ArgTypes...>::value>>      \
    AnyInvocable(F&& f) {                                                     \
      BaseType::template create<std::decay_t<F>>(std::forward<F>(f));         \
    }                                                                          \
                                                                               \
    template <class T, class... Args, class VT = std::decay_t<T>,              \
              class = std::enable_if_t<                                        \
                  std::is_move_constructible_v<VT> &&                          \
                  std::is_constructible_v<VT, Args...> &&                      \
                  std::is_invocable_r_v<_R, VT inv_quals, _ArgTypes...> &&       \
                  (!noex || std::is_nothrow_invocable_r_v<_R, VT inv_quals,     \
                                                          _ArgTypes...>)>>      \
    explicit AnyInvocable(std::in_place_type_t<T>, Args&&... args) {          \
      BaseType::template create<VT>(std::forward<Args>(args)...);             \
    }                                                                          \
                                                                               \
    template <                                                                 \
        class T, class U, class... Args, class VT = std::decay_t<T>,           \
        class = std::enable_if_t<                                              \
            std::is_move_constructible_v<VT> &&                                \
            std::is_constructible_v<VT, std::initializer_list<U>&, Args...> && \
            std::is_invocable_r_v<_R, VT inv_quals, _ArgTypes...> &&             \
            (!noex ||                                                          \
             std::is_nothrow_invocable_r_v<_R, VT inv_quals, _ArgTypes...>)>>    \
    explicit AnyInvocable(std::in_place_type_t<T>,                            \
                           std::initializer_list<U> il, Args&&... args) {      \
      BaseType::template create<VT>(il, std::forward<Args>(args)...);         \
    }                                                                          \
                                                                               \
    template <class F, class FDec = std::decay_t<F>>                           \
    std::enable_if_t<!std::is_same_v<FDec, AnyInvocable> &&                   \
                         std::is_move_constructible_v<FDec>,                   \
                     AnyInvocable&>                                           \
    operator=(F&& f) {                                                         \
      AnyInvocable{std::forward<F>(f)}.swap(*this);                           \
      return *this;                                                            \
    }                                                                          \
    template <class F>                                                         \
    AnyInvocable& operator=(std::reference_wrapper<F> f) {                    \
      AnyInvocable{f}.swap(*this);                                            \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    _R operator()(_ArgTypes... args) cv ref noexcept(noex) {                     \
      return BaseType::call(std::forward<_ArgTypes>(args)...);                 \
    }                                                                          \
  }

        // cv -> {`empty`, const}
        // ref -> {`empty`, &, &&}
        // noex -> {true, false}
        // inv_quals -> (is_empty(ref) ? & : ref)
        __SYSTEM_ANY_INVOCABLE(, , false, &);               // 000
        __SYSTEM_ANY_INVOCABLE(, , true, &);                // 001
        __SYSTEM_ANY_INVOCABLE(, &, false, &);              // 010
        __SYSTEM_ANY_INVOCABLE(, &, true, &);               // 011
        __SYSTEM_ANY_INVOCABLE(, &&, false, &&);            // 020
        __SYSTEM_ANY_INVOCABLE(, &&, true, &&);             // 021
        __SYSTEM_ANY_INVOCABLE(const, , false, const&);     // 100
        __SYSTEM_ANY_INVOCABLE(const, , true, const&);      // 101
        __SYSTEM_ANY_INVOCABLE(const, &, false, const&);    // 110
        __SYSTEM_ANY_INVOCABLE(const, &, true, const&);     // 111
        __SYSTEM_ANY_INVOCABLE(const, &&, false, const&&);  // 120
        __SYSTEM_ANY_INVOCABLE(const, &&, true, const&&);   // 121
#undef __SYSTEM_ANY_INVOCABLE

#define __SYSTEM_ANY_MEMBER_INVOCABLE(cv, ref, noex, inv_quals)                        \
  template <class _R, class _C, class... _ArgTypes>                                        \
  class AnyInvocable<_R(_C::*)(_ArgTypes...) cv ref noexcept(noex)>  final     \
      : public AnyInvocableMemberImpl<_R, _C, noex, _ArgTypes...> {          \
    using BaseType = AnyInvocableMemberImpl<_R, _C, noex, _ArgTypes...>;    \
                                                                               \
   public:                                                                     \
    using BaseType::BaseType;                                                \
    using ArgsType = std::tuple<_ArgTypes...>;                             \
    using ResultType = _R;                                                \
                                                                               \
    template <                                                                 \
        class F,                                                               \
        class = std::enable_if_t<can_convert<                      \
            AnyInvocable, F, noex, _R, F inv_quals, _C, _ArgTypes...>::value>>      \
    AnyInvocable(F&& f) {                                                     \
      BaseType::template create<std::decay_t<F>>(std::forward<F>(f));         \
    }                                                                          \
                                                                               \
    template <class T, class U, class... Args, class VT = std::decay_t<T>,              \
              class = std::enable_if_t<                                        \
                  std::is_move_constructible_v<VT> &&                          \
                  std::is_constructible_v<VT, Args...> &&                      \
                  std::is_invocable_r_v<_R, VT inv_quals, _C*, _ArgTypes...> &&       \
                  (!noex || std::is_nothrow_invocable_r_v<_R, VT inv_quals,     \
                                                          _C*, _ArgTypes...>)>>      \
    explicit AnyInvocable(std::in_place_type_t<T>, Args&&... args) {          \
      BaseType::template create<VT>(std::forward<Args>(args)...);             \
    }                                                                          \
                                                                               \
    template <                                                                 \
        class T, class U, class... Args, class VT = std::decay_t<T>,           \
        class = std::enable_if_t<                                              \
            std::is_move_constructible_v<VT> &&                                \
            std::is_constructible_v<VT, std::initializer_list<U>&, _C*, Args...> && \
            std::is_invocable_r_v<_R, VT inv_quals, _ArgTypes...> &&             \
            (!noex ||                                                          \
             std::is_nothrow_invocable_r_v<_R, VT inv_quals, _C*, _ArgTypes...>)>>    \
    explicit AnyInvocable(std::in_place_type_t<T>,                            \
                           std::initializer_list<U> il, Args&&... args) {      \
      BaseType::template create<VT>(il, std::forward<Args>(args)...);         \
    }                                                                          \
                                                                               \
    template <class F, class FDec = std::decay_t<F>>                           \
    std::enable_if_t<!std::is_same_v<FDec, AnyInvocable> &&                   \
                         std::is_move_constructible_v<FDec>,                   \
                     AnyInvocable&>                                           \
    operator=(F&& f) {                                                         \
      AnyInvocable{std::forward<F>(f)}.swap(*this);                           \
      return *this;                                                            \
    }                                                                          \
    template <class F>                                                         \
    AnyInvocable& operator=(std::reference_wrapper<F> f) {                    \
      AnyInvocable{f}.swap(*this);                                            \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    _R operator()(_C* ptr, _ArgTypes... args) cv ref noexcept(noex) {                     \
      return BaseType::call(ptr, std::forward<_ArgTypes>(args)...);                 \
    }                                                                          \
  }
        __SYSTEM_ANY_MEMBER_INVOCABLE(, , false, &);               // 000
        __SYSTEM_ANY_MEMBER_INVOCABLE(, , true, &);                // 001
        __SYSTEM_ANY_MEMBER_INVOCABLE(, &, false, &);              // 010
        __SYSTEM_ANY_MEMBER_INVOCABLE(, &, true, &);               // 011
        __SYSTEM_ANY_MEMBER_INVOCABLE(, &&, false, &&);            // 020
        __SYSTEM_ANY_MEMBER_INVOCABLE(, &&, true, &&);             // 021
        __SYSTEM_ANY_MEMBER_INVOCABLE(const, , false, const&);     // 100
        __SYSTEM_ANY_MEMBER_INVOCABLE(const, , true, const&);      // 101
        __SYSTEM_ANY_MEMBER_INVOCABLE(const, &, false, const&);    // 110
        __SYSTEM_ANY_MEMBER_INVOCABLE(const, &, true, const&);     // 111
        __SYSTEM_ANY_MEMBER_INVOCABLE(const, &&, false, const&&);  // 120
        __SYSTEM_ANY_MEMBER_INVOCABLE(const, &&, true, const&&);   // 121

#undef __SYSTEM_ANY_MEMBER_INVOCABLE
    }  // namespace Detail
}  // namespace System
