/*
Copyright 2016 University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef LIBCELLML_LIBCELLML_LOGGER_H_
#define LIBCELLML_LIBCELLML_LOGGER_H_

#include <string>

#include <libcellml/libcellml_export.h>
#include <libcellml/types.h>
#include <libcellml/loggererrors.h>

namespace libcellml {

/**
 * @brief The Logger class.
 *
 * Base class for all serialisable libCellML classes.
 */
class LIBCELLML_EXPORT Logger
{
public:
    Logger(); /**< Constructor */
    ~Logger(); /**< Destructor */
    Logger(const Logger &rhs); /**< Copy constructor */
    Logger(Logger &&rhs); /**< Move constructor */
    Logger& operator=(Logger rhs); /**< Assignment operator */

    /**
     * @brief Clear the errors from the logger.
     *
     * Clear the errors from the logger.
     */
    void clearErrors();

    /**
     * @brief addError
     * @param err
     */
    void addError(EntityErrorPtr err);

    /**
     * @brief Get the number of errors.
     *
     * Return the number of errors currently stored in the logger.
     *
     * @return The number of errors.
     */
    size_t errorCount() const;

    /**
     * @brief Get error at index.
     *
     * Returns a reference to an error at the index @p index.  If the  index
     * is not valid a @c std::out_of_range exception is thrown.
     *
     * @param index The index of the error to return (zero-based).
     *
     * @return A reference to the error at the given index.
     */
    EntityErrorPtr getError(size_t index);

private:
    void swap(Logger &rhs); /**< Swap method required for C++ 11 move semantics. */

    struct LoggerImpl; /**< Forward declaration for pImpl idiom. */
    LoggerImpl* mPimpl; /**< Private member to implementation pointer */
};

}

#endif /* LIBCELLML_LIBCELLML_LOGGER_H_ */


