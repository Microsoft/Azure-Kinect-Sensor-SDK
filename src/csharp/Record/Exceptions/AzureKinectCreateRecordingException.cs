//------------------------------------------------------------------------------
// <copyright file="AzureKinectCreateRecordingException.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Runtime.Serialization;

namespace Microsoft.Azure.Kinect.Sensor.Record.Exceptions
{
    /// <summary>
    /// Represents errors that occur when creating an Azure Kinect sensor recording.
    /// </summary>
    [Serializable]
    public class AzureKinectCreateRecordingException : AzureKinectRecordException
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="AzureKinectCreateRecordingException"/> class.
        /// </summary>
        public AzureKinectCreateRecordingException()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AzureKinectCreateRecordingException"/> class
        /// with a specified error message.
        /// </summary>
        /// <param name="message">The message that describes the error.</param>
        public AzureKinectCreateRecordingException(string message)
            : base(message)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AzureKinectCreateRecordingException"/> class
        /// with a specified error message and a reference to the inner exception that is the
        /// cause of this exception.
        /// </summary>
        /// <param name="message">
        /// The error message that explains the reason for the exception.
        /// </param>
        /// <param name="innerException">
        /// The exception that is the cause of the current exception, or a null reference
        /// (Nothing in Visual Basic) if no inner exception is specified.
        /// </param>
        public AzureKinectCreateRecordingException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AzureKinectCreateRecordingException"/> class
        /// with serialized data.
        /// </summary>
        /// <param name="info">
        /// The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">
        /// The <see cref="StreamingContext"/>System.Runtime.Serialization.StreamingContext that
        /// contains contextual information about the source or destination.
        /// </param>
        protected AzureKinectCreateRecordingException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AzureKinectCreateRecordingException"/> class
        /// with a specified error message.
        /// </summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="logMessages">
        /// The log messages that happened during the function call that generated this error.
        /// </param>
        protected AzureKinectCreateRecordingException(string message, ICollection<LogMessage> logMessages)
            : base(message, logMessages)
        {
        }

        /// <summary>
        /// Throws an <see cref="AzureKinectCreateRecordingException"/> if the result of the function
        /// is not a success.
        /// </summary>
        /// <param name="fileName">File name of the create.</param>
        /// <param name="function">The native function to call.</param>
        /// <typeparam name="T">The type of result to expect from the function call.</typeparam>
        internal static void ThrowIfNotSuccess<T>(string fileName, Func<T> function)
            where T : System.Enum
        {
            using (LoggingTracer tracer = new LoggingTracer(LogLevel.Warning, Logger.LogProvider, RecordLogger.LogProvider))
            {
                T result = function();
                if (!AzureKinectRecordException.IsSuccess(result))
                {
                    throw new AzureKinectCreateRecordingException($"fileName = \"{fileName}\"\r\nresult = {result}", tracer.LogMessages);
                }
            }
        }

        /// <summary>
        /// Throws an <see cref="AzureKinectCreateRecordingException"/> if the result of the function
        /// is not a success.
        /// </summary>
        /// <param name="fileName">File name of the create.</param>
        /// <param name="tracer">The tracer is that is capturing logging messages.</param>
        /// <param name="result">The result native function to call.</param>
        /// <typeparam name="T">The type of result to expect from the function call.</typeparam>
        internal static void ThrowIfNotSuccess<T>(string fileName, LoggingTracer tracer, T result)
            where T : System.Enum
        {
            if (!AzureKinectRecordException.IsSuccess(result))
            {
                throw new AzureKinectCreateRecordingException($"fileName = \"{fileName}\"\r\nresult = {result}", tracer.LogMessages);
            }
        }
    }
}
