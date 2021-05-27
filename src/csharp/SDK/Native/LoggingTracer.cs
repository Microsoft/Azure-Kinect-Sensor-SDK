//------------------------------------------------------------------------------
// <copyright file="LoggingTracer.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Threading;

namespace Microsoft.Azure.Kinect.Sensor
{
    /// <summary>
    /// Represents a tracer for capturing thread specific logging messages for tracing native calls
    /// into the Azure Kinect Sensor SDK.
    /// </summary>
    public class LoggingTracer : IDisposable
    {
        private readonly int threadId;
        private readonly LogLevel minLevel;

        private bool disposed;
        private List<LogMessage> messages;

        private ILoggingProvider[] loggingProviders;

        /// <summary>
        /// Initializes a new instance of the <see cref="LoggingTracer"/> class.
        /// </summary>
        public LoggingTracer()
            : this(LogLevel.Warning, Logger.LogProvider)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="LoggingTracer"/> class.
        /// </summary>
        /// <param name="minLevel">Minimum level of messages to capture.</param>
        /// <param name="loggingProvider">Set of logging providers to capture from.</param>
        public LoggingTracer(LogLevel minLevel, params ILoggingProvider[] loggingProvider)
        {
            this.messages = new List<LogMessage>();
            this.threadId = Thread.CurrentThread.ManagedThreadId;
            this.minLevel = minLevel;

            this.loggingProviders = (ILoggingProvider[])loggingProvider.Clone();

            foreach (ILoggingProvider provider in this.loggingProviders)
            {
                provider.LogMessage += this.Logger_LogMessage;
            }
        }

        /// <summary>
        /// Gets all of the messages that have occurred on this thread since the tracing began.
        /// </summary>
        public IList<LogMessage> LogMessages
        {
            get
            {
                if (this.disposed)
                {
                    throw new ObjectDisposedException(nameof(LoggingTracer));
                }

                if (this.threadId != Thread.CurrentThread.ManagedThreadId)
                {
                    throw new InvalidOperationException("LoggingTracer should only be access from the thread it was created on.");
                }

                return this.messages?.ToArray();
            }
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases unmanaged and - optionally - managed resources.
        /// </summary>
        /// <param name="disposing"><c>True</c> to release both managed and unmanaged resources; <c>False</c> to release only unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (disposing)
                {
                    foreach (ILoggingProvider provider in this.loggingProviders)
                    {
                        provider.LogMessage -= this.Logger_LogMessage;
                    }

                    this.loggingProviders = null;

                    // There are no longer any tracers on this thread. Clear the message list to allow the memory to be freed.
                    this.messages = null;
                }

                this.disposed = true;
            }
        }

        private void Logger_LogMessage(LogMessage logMessage)
        {
            if (this.threadId != Thread.CurrentThread.ManagedThreadId)
            {
                // The log messages aren't coming from this thread. Ignore them.
                return;
            }

            if (logMessage.LogLevel > this.minLevel)
            {
                // The log messages are too verbose. Ignore them.
                return;
            }

            this.messages.Add(logMessage);
        }
    }
}
