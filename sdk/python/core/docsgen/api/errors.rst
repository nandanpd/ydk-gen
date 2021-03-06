Errors
======

.. module:: ydk.errors
    :synopsis: YDK Exceptions

This module contains YDK Python errors classes. These errors are thrown in case of data not conforming to the yang model or due to a server-side error.

.. py:exception:: YPYError

    Bases: :exc:`exceptions.Exception`

    Base class for YPY Errors. The subclasses give a specialized view of the error that has occurred.

.. py:exception:: YPYModelError

    Bases: :exc:`ydk.errors.YPYError`

    Model Error. Thrown when a model constraint is violated.

.. py:exception:: YPYServiceProviderError

    Bases: :exc:`ydk.errors.YPYError`

    Exception for Service Provider. Thrown in case of a server-side error.

.. py:exception:: YPYClientError

    Bases: :exc:`ydk.errors.YPYError`

    Exception for client connection

.. py:exception:: YPYIllegalStateError

    Bases: :exc:`ydk.errors.YPYError`

    Illegal State Error. Thrown when an operation/service is invoked on an object that is not in the right state. Use the error_msg for the error.

.. py:exception:: YPYInvalidArgumentError

    Bases: :exc:`ydk.errors.YPYError`

    Invalid Argument. Use the error_msg for the error.

.. py:exception:: YPYOperationNotSupportedError

    Bases: :exc:`ydk.errors.YPYError`

    Operation Not Supported Error. Thrown when an operation is not supported.

.. py:exception:: YPYServiceError

    Bases: :exc:`ydk.errors.YPYError`

    Exception for Service Side Validation


